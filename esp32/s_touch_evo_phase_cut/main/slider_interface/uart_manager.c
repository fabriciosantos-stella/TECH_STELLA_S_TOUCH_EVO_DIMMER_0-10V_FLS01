/*
 *  uart_manager.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "uart_manager.h"
#include "hardware_config.h"
#include "status_led.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Private define ----------------------------------------------------------------------------------------
#define UART_MANAGER_TASK_STACK_SIZE            4096
#define UART_MANAGER_TASK_PRIORITY              5

#define UART_MANAGER_PERIOD_MS                  100
#define UART_MANAGER_RESPONSE_TIMEOUT_MS        30

#define UART_MANAGER_FRAME_SIZE                 UART_PROTOCOL_WRITE_RESPONSE_SIZE

// Private typedef ---------------------------------------------------------------------------------------
typedef enum
{
    UART_MANAGER_REQUEST_READ = 0,
    UART_MANAGER_REQUEST_WRITE
} UART_MANAGER_REQUEST_TYPE_t;

// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static SemaphoreHandle_t xUartManagerMutex = NULL;
static DEVICE_PARAMETERS_t xDesiredParameters;
static DEVICE_PARAMETERS_t xSlaveParameters;
static bool bDesiredParametersValid = false;
static bool bWritePending = false;
static bool bCommunicationOk = false;
static bool bParametersSynchronized = false;
static bool bTaskStarted = false;
static uint8_t u8PendingMasterFlags = 0;
static uint8_t u8PendingSlaveFlags = 0;
static uint8_t u8SequenceNumber = 0;

// Private function prototypes ---------------------------------------------------------------------------
static void UartManagerTask(void *pvParameters);
static esp_err_t UartManagerHardwareInit(void);
static bool UartManagerTransmit(const uint8_t *pu8Buffer, size_t xLength);
static bool UartManagerReceive(uint8_t *pu8Buffer, size_t xLength);
static bool UartManagerExecuteTransaction(UART_MANAGER_REQUEST_TYPE_t eRequestType, const DEVICE_PARAMETERS_t *pxParameters, uint8_t u8Sequence, DEVICE_PARAMETERS_t *pxReceivedParameters);
static bool UartManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParameters1, const DEVICE_PARAMETERS_t *pxParameters2);
static void UartManagerUpdateStatus(bool bStatus);

// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Função verifica se os parametros de duas estruturas são iguais ou se há alguma diferença
* @fn       static bool UartManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParameters1, const DEVICE_PARAMETERS_t *pxParameters2)
* @param    *pxParameters1        @brief ponteiro para a estrutura 1
* @param    *pxParameters2        @brief ponteiro para a estrutura 2
* @return   bool  @brief true se todos os parametros forem iguais, false se forem diferentes ou houver falha 
*********************************************************************************************************/
static bool UartManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParameters1, const DEVICE_PARAMETERS_t *pxParameters2)
{
    if ((pxParameters1 == NULL) || (pxParameters2 == NULL))
    {
        return false;
    }

    return (pxParameters1->u8DeviceOutputState == pxParameters2->u8DeviceOutputState) &&
           (pxParameters1->u8DimLevel == pxParameters2->u8DimLevel) &&
           (pxParameters1->u8MinDimLevel == pxParameters2->u8MinDimLevel) &&
           (pxParameters1->u8MaxDimLevel == pxParameters2->u8MaxDimLevel) &&
           (pxParameters1->u8NightModeStatus == pxParameters2->u8NightModeStatus) &&
           (pxParameters1->u8LockModeStatus == pxParameters2->u8LockModeStatus) &&
           (pxParameters1->u8PhaseCutMode == pxParameters2->u8PhaseCutMode);
}

/*********************************************************************************************************
* @brief    Função que inicializa os perifericos do sistema
* @fn       static esp_err_t UartManagerHardwareInit(void)
* @param    void        @brief void
* @return   esp_err_t   @brief status da resposta, sucesso ou falha 
*********************************************************************************************************/
static esp_err_t UartManagerHardwareInit(void)
{
    uart_config_t xUartConfig =
    {
        .baud_rate  = SLAVE_UART_BAUDRATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    esp_err_t xError = uart_param_config(SLAVE_UART_PORT, &xUartConfig);

    if (xError != ESP_OK)
    {
        return xError;
    }

    xError = uart_set_pin(SLAVE_UART_PORT, SLAVE_UART_TX_GPIO, SLAVE_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    if (xError != ESP_OK)
    {
        return xError;
    }

    return uart_driver_install(SLAVE_UART_PORT, SLAVE_UART_RX_BUFFER_SIZE, SLAVE_UART_TX_BUFFER_SIZE, 0, NULL, 0);
}

/*********************************************************************************************************
* @brief    Função que abstrai o driver para transmissão de uma mensagem pela uart
* @fn       static bool UartManagerTransmit(const uint8_t *pu8Buffer, size_t xLength)
* @param    *pu8Buffer        @brief ponteiro para o buffer de dados
* @param    xLength           @brief número de bytes da mensagem
* @return   bool       @brief true para sucesso ou false para falha
*********************************************************************************************************/
static bool UartManagerTransmit(const uint8_t *pu8Buffer, size_t xLength)
{
    if ((pu8Buffer == NULL) || (xLength == 0))
    {
        return false;
    }

    int iBytesWritten = uart_write_bytes(SLAVE_UART_PORT, pu8Buffer, xLength);

    if (iBytesWritten != (int)xLength)
    {
        return false;
    }

    return (uart_wait_tx_done(SLAVE_UART_PORT, pdMS_TO_TICKS(UART_MANAGER_RESPONSE_TIMEOUT_MS)) == ESP_OK);
}

/*********************************************************************************************************
* @brief    Função que abstrai o driver para recepção de uma mensagem pela uart
* @fn       static bool UartManagerReceive(uint8_t *pu8Buffer, size_t xLength)
* @param    *pu8Buffer        @brief ponteiro para o buffer de dados
* @param    xLength           @brief número de bytes da mensagem
* @return   bool       @brief true para sucesso ou false para falha
*********************************************************************************************************/
static bool UartManagerReceive(uint8_t *pu8Buffer, size_t xLength)
{
    if ((pu8Buffer == NULL) || (xLength == 0))
    {
        return false;
    }

    TickType_t xStartTime = xTaskGetTickCount();
    TickType_t xTimeout = pdMS_TO_TICKS(UART_MANAGER_RESPONSE_TIMEOUT_MS);
    size_t xReceived = 0;

    while (xReceived < xLength)
    {
        TickType_t xElapsed = xTaskGetTickCount() - xStartTime;

        if (xElapsed >= xTimeout)
        {
            return false;
        }

        TickType_t xRemaining = xTimeout - xElapsed;
        int iBytesRead = uart_read_bytes(SLAVE_UART_PORT, &pu8Buffer[xReceived], 1, xRemaining);

        if (iBytesRead != 1)
        {
            return false;
        }

        if (xReceived == 0)
        {
            if (pu8Buffer[0] != UART_PROTOCOL_START_BYTE)
            {
                continue;
            }
        }

        xReceived++;
    }

    return true;
}

/*********************************************************************************************************
* @brief    Função que gerencia o envio e o recebimento das mensagens pela uart
* @fn       static bool UartManagerExecuteTransaction(UART_MANAGER_REQUEST_TYPE_t eRequestType, const DEVICE_PARAMETERS_t *pxParameters)
* @param    eRequestType            @brief  tipo de mensagem, leitura ou escrita
* @param    *pxParameters           @brief  ponteiro para a estrutura de parametros
* @param    u8Sequence              @brief  numero sequêncial da mensagem
* @param    *pxReceivedParameters   @brief  ponteiro com os parametros recebidos
* @return   bool       @brief true para sucesso, false para falha
*********************************************************************************************************/
static bool UartManagerExecuteTransaction(UART_MANAGER_REQUEST_TYPE_t eRequestType, const DEVICE_PARAMETERS_t *pxParameters, uint8_t u8Sequence, DEVICE_PARAMETERS_t *pxReceivedParameters)
{
    uint8_t u8TxBuffer[UART_MANAGER_FRAME_SIZE];
    uint8_t u8RxBuffer[UART_MANAGER_FRAME_SIZE];
    size_t xTxLength;
    uint8_t u8ExpectedCommand;

    if (pxReceivedParameters == NULL)
    {
        return false;
    }

    if (uart_flush_input(SLAVE_UART_PORT) != ESP_OK) // limpa possiveis dados previamente recebidos
    {
        return false;
    }

    if (eRequestType == UART_MANAGER_REQUEST_WRITE) // monta mensagem de escrita
    {
        if (pxParameters == NULL)
        {
            return false;
        }

        if (!UartProtocolBuildWriteRequest(u8Sequence, pxParameters, u8TxBuffer, sizeof(u8TxBuffer)))
        {
            return false;
        }

        xTxLength = UART_PROTOCOL_WRITE_REQUEST_SIZE;
        u8ExpectedCommand = UART_PROTOCOL_WRITE_COMMAND;
    }
    else // monta mensagem de leitura
    {
        if (!UartProtocolBuildReadRequest(u8Sequence, u8TxBuffer, sizeof(u8TxBuffer)))
        {
            return false;
        }

        xTxLength = UART_PROTOCOL_READ_REQUEST_SIZE;
        u8ExpectedCommand = UART_PROTOCOL_READ_COMMAND;
    }

    if (!UartManagerTransmit(u8TxBuffer, xTxLength)) // envia a mensagem
    {
        return false;
    }

    if (!UartManagerReceive(u8RxBuffer, UART_MANAGER_FRAME_SIZE)) // recebe a mensagem de resposta
    {
        return false;
    }

    if (!UartProtocolValidateFrame(u8RxBuffer, UART_MANAGER_FRAME_SIZE, u8ExpectedCommand, u8Sequence)) // valida a mensagemd e resposta
    {
        return false;
    }

    if (!UartProtocolParseParameters(u8RxBuffer, UART_MANAGER_FRAME_SIZE, pxReceivedParameters)) // processa os parametros recebidos
    {
        return false;
    }

    if (eRequestType == UART_MANAGER_REQUEST_READ) // valida as flags de função de acorco com sua origem
    {
        if (pxReceivedParameters->u8FunctionFlags & (uint8_t)(~UART_PROTOCOL_MASTER_COMMAND_MASK))
        {
            return false;
        }
    }
    else
    {
        if (pxReceivedParameters->u8FunctionFlags & (uint8_t)(~UART_PROTOCOL_SLAVE_CONFIRMATION_MASK))
        {
            return false;
        }
    }

    return true;
}

/*********************************************************************************************************
* @brief    Função que atualiza o status da comunicação com o escravo
* @fn       static void UartManagerUpdateStatus(bool bStatus)
* @param    bStatus       @brief flag com o status da comunicação
* @return   void       @brief void
*********************************************************************************************************/
static void UartManagerUpdateStatus(bool bStatus)
{
    if (xUartManagerMutex != NULL)
    {
        if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE)
        {
            bCommunicationOk = bStatus;
            xSemaphoreGive(xUartManagerMutex);
        }
    }

    if (bStatus)
    {
        StatusLedSetMode(STATUS_LED_MODE_NORMAL);
    }
    else
    {
        StatusLedSetMode(STATUS_LED_SLAVE_COMM_ERROR);
    }
}

/*********************************************************************************************************
* @brief    Tarefa de gerenciamento da comunicação uart
* @fn       static void UartManagerTask(void *pvParameters)
* @param    *pvParameters       @brief Ponteiro para os parametros da tarefa
* @return   void       @brief void
*********************************************************************************************************/
static void UartManagerTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    DEVICE_PARAMETERS_t xTransactionParameters;
    DEVICE_PARAMETERS_t xReceivedParameters;
    UART_MANAGER_REQUEST_TYPE_t eRequestType;
    uint8_t u8TransactionFlags;
    uint8_t u8AcknowledgedCommands;
    bool bTransactionOk;
    bool bParametersEqual;

    while (1)
    {
        eRequestType = UART_MANAGER_REQUEST_READ;
        u8TransactionFlags = 0;

        if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE) // seleciona a proxima transação e copia os dados
        {
            if (bDesiredParametersValid && (bWritePending || (u8PendingMasterFlags != 0)))
            {
                eRequestType                            = UART_MANAGER_REQUEST_WRITE;
                xTransactionParameters                  = xDesiredParameters;
                u8TransactionFlags                      = u8PendingMasterFlags;
                xTransactionParameters.u8FunctionFlags  = u8TransactionFlags;
            }

            xSemaphoreGive(xUartManagerMutex);
        }

        // Executa a transação fora do mutex
        bTransactionOk = UartManagerExecuteTransaction(eRequestType, (eRequestType == UART_MANAGER_REQUEST_WRITE) ? &xTransactionParameters : NULL, u8SequenceNumber, &xReceivedParameters);

        if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE) // pega o semáforo para processar a resposta
        {
            if (bTransactionOk)
            {
                xSlaveParameters = xReceivedParameters;

                if (eRequestType == UART_MANAGER_REQUEST_READ)
                {
                    u8PendingSlaveFlags |= (xReceivedParameters.u8FunctionFlags & UART_PROTOCOL_MASTER_COMMAND_MASK); // registra a origem da requisição sendo do escravo
                }
                else
                {
                    u8AcknowledgedCommands = (uint8_t)((xReceivedParameters.u8FunctionFlags >> 2) & u8TransactionFlags); // converte as confirmações do escravo no formato do mestre
                    u8PendingMasterFlags &= (uint8_t)(~u8AcknowledgedCommands); // limpa apenas flags confirmadas pelo escravo
                    u8PendingSlaveFlags &= (uint8_t)(~u8AcknowledgedCommands); // limpa requisições correspondentes do escravo
                }

                if (eRequestType == UART_MANAGER_REQUEST_READ)
                {
                    u8PendingSlaveFlags |= (xReceivedParameters.u8FunctionFlags & UART_PROTOCOL_MASTER_COMMAND_MASK); // Registra as solicitações originadas no escravo

                    if (bDesiredParametersValid) // Atualiza somente o status de sincronização. Nunca agenda uma escrita após uma leitura
                    {
                        bParametersEqual = UartManagerParametersEqual(&xDesiredParameters, &xSlaveParameters);
                        bParametersSynchronized = bParametersEqual;
                    }
                    else
                    {
                        bParametersSynchronized = false;
                    }
                }
                else
                {
                    u8AcknowledgedCommands = (uint8_t)((xReceivedParameters.u8FunctionFlags >> 2) & u8TransactionFlags); // Processa as confirmações dos comandos enviados pelo mestre
                    u8PendingMasterFlags &= (uint8_t)(~u8AcknowledgedCommands);
                    u8PendingSlaveFlags &= (uint8_t)(~u8AcknowledgedCommands);
                    
                    if (bDesiredParametersValid) // Após WRITE, verifica se os parâmetros foram efetivamente sincronizados
                    {
                        bParametersEqual = UartManagerParametersEqual(&xDesiredParameters, &xSlaveParameters);
                        bParametersSynchronized = bParametersEqual;
                        bWritePending = !bParametersEqual;
                    }
                    else
                    {
                        bParametersSynchronized = false;
                    }
                }
            }
            else
            {
                bParametersSynchronized = false; // mantem as operações pendentes para retransmissão
            }

            xSemaphoreGive(xUartManagerMutex);
        }

        UartManagerUpdateStatus(bTransactionOk); // atualiza o status da transação
        u8SequenceNumber++;  // incrementa o contador sequêncial
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(UART_MANAGER_PERIOD_MS)); // reagenda proxima execução da tarefa
    }
}

/*********************************************************************************************************
* @brief    Função que inicializa as rotinas de envio e recepção da comunicação uart com o escravo
* @fn       esp_err_t UartManagerInit(void)
* @param    void        @brief void
* @return   esp_err_t   @brief status da execução da função, sucesso ou falha
*********************************************************************************************************/
esp_err_t UartManagerInit(void)
{
    if (xUartManagerMutex != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    xUartManagerMutex = xSemaphoreCreateMutex();

    if (xUartManagerMutex == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t xError = UartManagerHardwareInit();

    if (xError != ESP_OK)
    {
        vSemaphoreDelete(xUartManagerMutex);
        xUartManagerMutex = NULL;

        return xError;
    }

    xDesiredParameters      = (DEVICE_PARAMETERS_t){0};
    xSlaveParameters        = (DEVICE_PARAMETERS_t){0};
    bDesiredParametersValid = false;
    bWritePending           = false;
    bCommunicationOk        = false;
    bParametersSynchronized = false;
    bTaskStarted            = false;
    u8PendingMasterFlags    = 0;
    u8PendingSlaveFlags     = 0;
    u8SequenceNumber        = 0;

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que cria a tarefa de comunicação serial
* @fn       esp_err_t UartManagerStart(void)
* @param    void        @brief void
* @return   esp_err_t   @brief status da execução da função, sucesso ou falha
*********************************************************************************************************/
esp_err_t UartManagerStart(void)
{
    if (xUartManagerMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (bTaskStarted)
    {
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t xResult = xTaskCreate(UartManagerTask, "UartManagerTask", UART_MANAGER_TASK_STACK_SIZE, NULL, UART_MANAGER_TASK_PRIORITY, NULL);

    if (xResult != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    bTaskStarted = true;

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que recebe uma solicitação de atualização dos parâmetros do escravo
* @fn       esp_err_t UartManagerWriteParameters(const DEVICE_PARAMETERS_t *pxParameters)
* @param    *pxParameters       @brief ponteiro para a estrutura de parametros
* @return   esp_err_t           @brief status da execução da função, sucesso ou falha
*********************************************************************************************************/
esp_err_t UartManagerWriteParameters(const DEVICE_PARAMETERS_t *pxParameters)
{
    if (pxParameters == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xUartManagerMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (pxParameters->u8FunctionFlags & (uint8_t)(~UART_PROTOCOL_MASTER_COMMAND_MASK))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    xDesiredParameters                  = *pxParameters;
    xDesiredParameters.u8FunctionFlags  = 0; 
    u8PendingMasterFlags                |= pxParameters->u8FunctionFlags;
    bDesiredParametersValid             = true;
    bWritePending                       = true;
    bParametersSynchronized             = false;

    xSemaphoreGive(xUartManagerMutex);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que solicita a leitura de parametros do escravo
* @fn       esp_err_t UartManagerReadParameters(void)
* @param    void        @brief void
* @return   esp_err_t   @brief status da execução da função, sucesso ou falha
*********************************************************************************************************/
esp_err_t UartManagerReadParameters(void)
{
    if (xUartManagerMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que carrega os parametros do escravo na estrutura de parametros
* @fn       bool UartManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters)
* @param    *pxParameters        @brief ponteiro para a estrutura de parametros
* @return   bool   @brief true para sucesso, false para falha
*********************************************************************************************************/
bool UartManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters)
{
    if ((pxParameters == NULL) || (xUartManagerMutex == NULL))
    {
        return false;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }

    *pxParameters = xSlaveParameters;
    xSemaphoreGive(xUartManagerMutex);

    return true;
}

/*********************************************************************************************************
* @brief    Função que atualiza o status da comunicação com o escravo
* @fn       bool UartManagerIsCommunicationOk(void)
* @param    void       @brief void
* @return   bool       @brief true para sucesso, false para falha
*********************************************************************************************************/
bool UartManagerIsCommunicationOk(void)
{
    bool bResult = false;

    if (xUartManagerMutex == NULL)
    {
        return false;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE)
    {
        bResult = bCommunicationOk;
        xSemaphoreGive(xUartManagerMutex);
    }

    return bResult;
}

/*********************************************************************************************************
* @brief    Função que retorna o status da sincronização dos dados entre mestre e escravo
* @fn       bool UartManagerAreParametersSynchronized(void)
* @param    void       @brief void
* @return   bool       @brief true se os dados estiverem sincronizados, false se não estiverem
*********************************************************************************************************/
bool UartManagerAreParametersSynchronized(void)
{
    bool bResult = false;

    if (xUartManagerMutex == NULL)
    {
        return false;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE)
    {
        bResult = bParametersSynchronized;
        xSemaphoreGive(xUartManagerMutex);
    }

    return bResult;
}

/*********************************************************************************************************
* @brief    Retorna as solicitações pendentes originadas no escravo
* @param    pu8FunctionFlags    Flags de solicitações pendentes
* @return   esp_err_t           Status da operação
*********************************************************************************************************/
esp_err_t UartManagerGetPendingSlaveRequests(uint8_t *pu8FunctionFlags)
{
    if (pu8FunctionFlags == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xUartManagerMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    *pu8FunctionFlags = u8PendingSlaveFlags;
    xSemaphoreGive(xUartManagerMutex);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que verifica se o mestre pode aceitar atualizações de parâmetros originadas no escravo
* @fn       bool UartManagerCanAcceptSlaveParameters(void)
* @param    void    @brief void
* @return   bool    @brief true se não houver sincronização ou comando do mestre pendente
*********************************************************************************************************/
bool UartManagerCanAcceptSlaveParameters(void)
{
    bool bResult = false;

    if (xUartManagerMutex == NULL)
    {
        return false;
    }

    if (xSemaphoreTake(xUartManagerMutex, portMAX_DELAY) == pdTRUE)
    {
        bResult = bDesiredParametersValid && !bWritePending && (u8PendingMasterFlags == 0);
        xSemaphoreGive(xUartManagerMutex);
    }

    return bResult;
}
