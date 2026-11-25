/*
 *  parameter_manager.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "esp_log.h"
#include "storage_manager.h"
#include "slider_interface/uart_manager.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

// Private define ----------------------------------------------------------------------------------------
#define PARAMETER_NVS_NAMESPACE             "device_cfg"
#define PARAMETER_NVS_KEY                   "params"
#define PARAMETER_STORAGE_SIZE              7

#define DEVICE_DEFAULT_OUTPUT_STATE         0
#define DEVICE_DEFAULT_DIM_LEVEL            5
#define DEVICE_DEFAULT_MIN_DIM_LEVEL        2
#define DEVICE_DEFAULT_MAX_DIM_LEVEL        8
#define DEVICE_DEFAULT_NIGHT_MODE           1
#define DEVICE_DEFAULT_LOCK_MODE            0
#define DEVICE_DEFAULT_PHASE_CUT_MODE       0

#define DEVICE_DIM_LEVEL_MIN                0
#define DEVICE_DIM_LEVEL_MAX                11

#define PARAMETER_TASK_STACK_SIZE           3072
#define PARAMETER_TASK_PRIORITY             3
#define PARAMETER_SAVE_DELAY_MS             2000

#define PARAMETER_POLL_PERIOD_MS            100

// Private typedef ---------------------------------------------------------------------------------------
typedef enum
{
    PARAM_STORAGE_OUTPUT_STATE = 0,
    PARAM_STORAGE_DIM_LEVEL,
    PARAM_STORAGE_MIN_DIM_LEVEL,
    PARAM_STORAGE_MAX_DIM_LEVEL,
    PARAM_STORAGE_NIGHT_MODE,
    PARAM_STORAGE_LOCK_MODE,
    PARAM_STORAGE_PHASE_CUT_MODE
} PARAMETER_STORAGE_IDX_t;

// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static DEVICE_PARAMETERS_t xDeviceParameters;
static DEVICE_PARAMETERS_t xPersistedParameters;
static SemaphoreHandle_t xParameterMutex = NULL;
static TaskHandle_t xParameterTaskHandle = NULL;

// Private function prototypes ---------------------------------------------------------------------------
static void StorageManagerLoadDefaults(void);
static bool StorageManagerValidate(const DEVICE_PARAMETERS_t *pxParameters);
static bool StorageManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParametersA, const DEVICE_PARAMETERS_t *pxParametersB);
static void StorageManagerSerialize(const DEVICE_PARAMETERS_t *pxParameters, uint8_t *pu8Buffer);
static void StorageManagerDeserialize(const uint8_t *pu8Buffer, DEVICE_PARAMETERS_t *pxParameters);
static esp_err_t StorageManagerSave(const DEVICE_PARAMETERS_t *pxParameters);
static void StorageManagerTask(void *pvParameters);

// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Função compara os parametros configuráveis
* @fn       static bool StorageManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParametersA, const DEVICE_PARAMETERS_t *pxParametersB)
* @param    *pxParametersA      @brief ponteiro para a estrutura A
* @param    *pxParametersB      @brief ponteiro para a estrutura B
* @return   bool      @brief true se todos os dados foram identicos, false se não forem
*********************************************************************************************************/
static bool StorageManagerParametersEqual(const DEVICE_PARAMETERS_t *pxParametersA, const DEVICE_PARAMETERS_t *pxParametersB)
{
    if ((pxParametersA == NULL) || (pxParametersB == NULL))
    {
        return false;
    }

    return (pxParametersA->u8DeviceOutputState == pxParametersB->u8DeviceOutputState) &&
           (pxParametersA->u8DimLevel == pxParametersB->u8DimLevel) &&
           (pxParametersA->u8MinDimLevel == pxParametersB->u8MinDimLevel) &&
           (pxParametersA->u8MaxDimLevel == pxParametersB->u8MaxDimLevel) &&
           (pxParametersA->u8NightModeStatus == pxParametersB->u8NightModeStatus) &&
           (pxParametersA->u8LockModeStatus == pxParametersB->u8LockModeStatus) &&
           (pxParametersA->u8PhaseCutMode == pxParametersB->u8PhaseCutMode);
}

/*********************************************************************************************************
* @brief    Função que carrega os parametros padrão
* @fn       static void StorageManagerLoadDefaults(void)
* @param    void     @brief void
* @return   void     @brief void
*********************************************************************************************************/
static void StorageManagerLoadDefaults(void)
{
    memset(&xDeviceParameters, 0, sizeof(xDeviceParameters));

    xDeviceParameters.u8DeviceOutputState = DEVICE_DEFAULT_OUTPUT_STATE;
    xDeviceParameters.u8DimLevel          = DEVICE_DEFAULT_DIM_LEVEL;
    xDeviceParameters.u8MinDimLevel       = DEVICE_DEFAULT_MIN_DIM_LEVEL;
    xDeviceParameters.u8MaxDimLevel       = DEVICE_DEFAULT_MAX_DIM_LEVEL;
    xDeviceParameters.u8NightModeStatus   = DEVICE_DEFAULT_NIGHT_MODE;
    xDeviceParameters.u8LockModeStatus    = DEVICE_DEFAULT_LOCK_MODE;
    xDeviceParameters.u8PhaseCutMode      = DEVICE_DEFAULT_PHASE_CUT_MODE;
}

/*********************************************************************************************************
* @brief    Função que valida os parametros configuráveis
* @fn       static bool StorageManagerValidate(const DEVICE_PARAMETERS_t *pxParameters)
* @param    *pxParameters     @brief ponteiro para a estrutura de parametros do dispositivo
* @return   bool     @brief true se os parametros são válidos, false se algum não for
*********************************************************************************************************/
static bool StorageManagerValidate(const DEVICE_PARAMETERS_t *pxParameters)
{
    if (pxParameters == NULL)
    {
        return false;
    }

    if (pxParameters->u8DeviceOutputState > 1)
    {
        return false;
    }

    if ((pxParameters->u8DimLevel > DEVICE_DIM_LEVEL_MAX) || (pxParameters->u8MinDimLevel > DEVICE_DIM_LEVEL_MAX) || (pxParameters->u8MaxDimLevel > DEVICE_DIM_LEVEL_MAX))
    {
        return false;
    }

    if (pxParameters->u8MinDimLevel > pxParameters->u8MaxDimLevel)
    {
        return false;
    }

    if ((pxParameters->u8NightModeStatus > 1) || (pxParameters->u8LockModeStatus > 1) || (pxParameters->u8PhaseCutMode > 1))
    {
        return false;
    }

    return true;
}

/*********************************************************************************************************
* @brief    Função que serializa os parâmetros para armazenamento na NVS
* @fn       static void StorageManagerSerialize(const DEVICE_PARAMETERS_t *pxParameters, uint8_t *pu8Buffer)
* @param    *pxParameters     @brief ponteiro para a estrutura de parametros do dispositivo
* @param    *pu8Buffer      @brief ponteiro para o que recebe os parametros
* @return   void     @brief void
*********************************************************************************************************/
static void StorageManagerSerialize(const DEVICE_PARAMETERS_t *pxParameters, uint8_t *pu8Buffer)
{
    pu8Buffer[PARAM_STORAGE_OUTPUT_STATE]   = pxParameters->u8DeviceOutputState;
    pu8Buffer[PARAM_STORAGE_DIM_LEVEL]      = pxParameters->u8DimLevel;
    pu8Buffer[PARAM_STORAGE_MIN_DIM_LEVEL]  = pxParameters->u8MinDimLevel;
    pu8Buffer[PARAM_STORAGE_MAX_DIM_LEVEL]  = pxParameters->u8MaxDimLevel;
    pu8Buffer[PARAM_STORAGE_NIGHT_MODE]     = pxParameters->u8NightModeStatus;
    pu8Buffer[PARAM_STORAGE_LOCK_MODE]      = pxParameters->u8LockModeStatus;
    pu8Buffer[PARAM_STORAGE_PHASE_CUT_MODE] = pxParameters->u8PhaseCutMode;
}

/*********************************************************************************************************
* @brief    Função que recupera os parametros de um buffer NVS
* @fn       static void StorageManagerDeserialize(const uint8_t *pu8Buffer, DEVICE_PARAMETERS_t *pxParameters)
* @param    *pu8Buffer      @brief ponteiro para o que recebe os parametros
* @param    *pxParameters     @brief ponteiro para a estrutura de parametros do dispositivo
* @return   void     @brief void
*********************************************************************************************************/
static void StorageManagerDeserialize(const uint8_t *pu8Buffer, DEVICE_PARAMETERS_t *pxParameters)
{
    memset(pxParameters, 0, sizeof(*pxParameters));
    pxParameters->u8DeviceOutputState = pu8Buffer[PARAM_STORAGE_OUTPUT_STATE];
    pxParameters->u8DimLevel          = pu8Buffer[PARAM_STORAGE_DIM_LEVEL];
    pxParameters->u8MinDimLevel       = pu8Buffer[PARAM_STORAGE_MIN_DIM_LEVEL];
    pxParameters->u8MaxDimLevel       = pu8Buffer[PARAM_STORAGE_MAX_DIM_LEVEL];
    pxParameters->u8NightModeStatus   = pu8Buffer[PARAM_STORAGE_NIGHT_MODE];
    pxParameters->u8LockModeStatus    = pu8Buffer[PARAM_STORAGE_LOCK_MODE];
    pxParameters->u8PhaseCutMode      = pu8Buffer[PARAM_STORAGE_PHASE_CUT_MODE];
}

/*********************************************************************************************************
* @brief    Função que grava os parametros na NVS
* @fn       static esp_err_t StorageManagerSave(const DEVICE_PARAMETERS_t *pxParameters)
* @param    *pxParameters     @brief ponteiro para a estrutura de parametros do dispositivo
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
static esp_err_t StorageManagerSave(const DEVICE_PARAMETERS_t *pxParameters)
{
    uint8_t u8StorageBuffer[PARAMETER_STORAGE_SIZE];
    nvs_handle_t xNvsHandle;
    esp_err_t xError;

    if (!StorageManagerValidate(pxParameters))
    {
        return ESP_ERR_INVALID_ARG;
    }

    StorageManagerSerialize(pxParameters, u8StorageBuffer);
    xError = nvs_open(PARAMETER_NVS_NAMESPACE, NVS_READWRITE, &xNvsHandle);

    if (xError != ESP_OK)
    {
        return xError;
    }

    xError = nvs_set_blob(xNvsHandle, PARAMETER_NVS_KEY, u8StorageBuffer, sizeof(u8StorageBuffer));

    if (xError == ESP_OK)
    {
        xError = nvs_commit(xNvsHandle);
    }

    nvs_close(xNvsHandle);

    return xError;
}

/*********************************************************************************************************
* @brief    Tarefa responsável pela atualização e persistência dos parâmetros
* @fn       static void StorageManagerTask(void *pvParameters)
* @param    *pvParameters      @brief ponteiro para os parametros da tarefa
* @return   void      @brief void
*********************************************************************************************************/
static void StorageManagerTask(void *pvParameters)
{
    DEVICE_PARAMETERS_t xSlaveParameters;
    DEVICE_PARAMETERS_t xParametersToSave;
    esp_err_t xError;
    TickType_t xPollPeriod = pdMS_TO_TICKS(PARAMETER_POLL_PERIOD_MS);
    TickType_t xSaveDelay = pdMS_TO_TICKS(PARAMETER_SAVE_DELAY_MS);
    TickType_t xLastChangeTime = 0;
    TickType_t xCurrentTime;
    bool bSavePending = false;
    bool bSlaveParametersUpdated;

    (void)pvParameters;

    while (1)
    {        
        if (ulTaskNotifyTake(pdTRUE, xPollPeriod) > 0) // Aguarda uma notificação ou o próximo período de consulta
        {
            bSavePending = true;
            xLastChangeTime = xTaskGetTickCount();
        }

        bSlaveParametersUpdated = false;
        
        if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) == pdTRUE) // Consulta os parâmetros recebidos do escravo
        {
            // somente aceita alterações quando não houver operações do mestre pendentes
            if (UartManagerCanAcceptSlaveParameters() && UartManagerGetParameters(&xSlaveParameters))
            {
                xSlaveParameters.u8FunctionFlags = 0;

                if (StorageManagerValidate(&xSlaveParameters) && !StorageManagerParametersEqual(&xDeviceParameters, &xSlaveParameters))
                {
                    xDeviceParameters = xSlaveParameters;
                    bSlaveParametersUpdated = true;
                }
            }

            xSemaphoreGive(xParameterMutex);
        }

        if (bSlaveParametersUpdated)
        {
            bSavePending = true;
            xLastChangeTime = xTaskGetTickCount();
        }
        
        if (!bSavePending) // Verifica se existem parâmetros diferentes dos valores persistidos
        {
            if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) == pdTRUE)
            {
                if (!StorageManagerParametersEqual(&xDeviceParameters, &xPersistedParameters))
                {
                    bSavePending = true;
                    xLastChangeTime = xTaskGetTickCount();
                }

                xSemaphoreGive(xParameterMutex);
            }
        }

        xCurrentTime = xTaskGetTickCount(); // Verifica se o período de debounce terminou

        if (bSavePending && ((TickType_t)(xCurrentTime - xLastChangeTime) >= xSaveDelay))
        {
            if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
            {
                continue;
            }

            if (StorageManagerParametersEqual(&xDeviceParameters, &xPersistedParameters))
            {
                bSavePending = false;
                xSemaphoreGive(xParameterMutex);
                continue;
            }

            xParametersToSave = xDeviceParameters;
            xSemaphoreGive(xParameterMutex);   
            xError = StorageManagerSave(&xParametersToSave); // Persiste os parâmetros na NVS

            if (xError == ESP_OK)
            {
                if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) == pdTRUE)
                {
                    xPersistedParameters = xParametersToSave;
                    bSavePending = !StorageManagerParametersEqual(&xDeviceParameters, &xPersistedParameters);

                    if (bSavePending)
                    {
                        xLastChangeTime = xTaskGetTickCount();
                    }

                    xSemaphoreGive(xParameterMutex);
                }
            }
            else
            {
                
                xLastChangeTime = xTaskGetTickCount(); // Em caso de falha, tenta novamente após o período de debounce
            }
        }
    }
}

/*********************************************************************************************************
* @brief    Função que inicializa o gerenciamento dos parametros
* @fn       esp_err_t StorageManagerInit(void)
* @param    void            @brief void
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerInit(void)
{
    esp_err_t xError;
    bool bLoadDefaults = false;
    uint8_t u8StorageBuffer[PARAMETER_STORAGE_SIZE];
    size_t xStorageSize = sizeof(u8StorageBuffer);
    nvs_handle_t xNvsHandle;

    if (xParameterMutex != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    xError = nvs_flash_init();

    if (xError != ESP_OK)
    {
        return xError;
    }

    xParameterMutex = xSemaphoreCreateMutex();

    if (xParameterMutex == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    StorageManagerLoadDefaults();
    xError = nvs_open(PARAMETER_NVS_NAMESPACE, NVS_READONLY, &xNvsHandle);

    if (xError == ESP_ERR_NVS_NOT_FOUND)
    {
        bLoadDefaults = true;
    }
    else if (xError != ESP_OK)
    {
        return xError;
    }
    else
    {
        xError = nvs_get_blob(xNvsHandle, PARAMETER_NVS_KEY, u8StorageBuffer, &xStorageSize);
        nvs_close(xNvsHandle);

        if ((xError == ESP_OK) && (xStorageSize == sizeof(u8StorageBuffer)))
        {
            StorageManagerDeserialize(u8StorageBuffer, &xDeviceParameters);

            if (!StorageManagerValidate(&xDeviceParameters))
            {
                bLoadDefaults = true;
            }
        }
        else
        {
            bLoadDefaults = true;
        }
    }

    if (bLoadDefaults)
    {
        StorageManagerLoadDefaults();
        xError = StorageManagerSave(&xDeviceParameters);

        if (xError != ESP_OK)
        {
            return xError;
        }
    }

    xPersistedParameters = xDeviceParameters;

    if (xTaskCreate(StorageManagerTask, "StorageManager", PARAMETER_TASK_STACK_SIZE, NULL, PARAMETER_TASK_PRIORITY, &xParameterTaskHandle) != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que obtem os parametros atuais
* @fn       esp_err_t StorageManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters)
* @param    *pxParameters      @brief ponteiro para a estrutura de parametros
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters)
{
    if ((pxParameters == NULL) || (xParameterMutex == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    *pxParameters = xDeviceParameters;
    xSemaphoreGive(xParameterMutex);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que atualiza os parametros a partir do mestre
* @fn       esp_err_t StorageManagerSetParameters(const DEVICE_PARAMETERS_t *pxParameters)
* @param    *pvParameters      @brief ponteiro para a estrutura de parametros
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerSetParameters(const DEVICE_PARAMETERS_t *pxParameters)
{
    DEVICE_PARAMETERS_t xNewParameters;
    esp_err_t xError;

    if ((pxParameters == NULL) || (xParameterMutex == NULL) || (xParameterTaskHandle == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    xNewParameters = *pxParameters;
    xNewParameters.u8FunctionFlags = 0;

    if (!StorageManagerValidate(&xNewParameters))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    if (StorageManagerParametersEqual(&xDeviceParameters, &xNewParameters))
    {
        xSemaphoreGive(xParameterMutex);
        return ESP_OK;
    }
    
    xError = UartManagerWriteParameters(&xNewParameters); // Registra a atualização no UartManager antes de liberar o mutex

    if (xError != ESP_OK)
    {
        xSemaphoreGive(xParameterMutex);
        return xError;
    }

    xDeviceParameters = xNewParameters;
    xSemaphoreGive(xParameterMutex);
    xTaskNotifyGive(xParameterTaskHandle);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função que restaura as configurações de fábrica
* @fn       esp_err_t StorageManagerFactoryReset(void)
* @param    void     @brief void
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerFactoryReset(void)
{
    DEVICE_PARAMETERS_t xFactoryParameters;
    esp_err_t xError;

    if (xParameterMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    StorageManagerLoadDefaults();
    xFactoryParameters = xDeviceParameters;
    xError = StorageManagerSave(&xFactoryParameters);

    if (xError != ESP_OK)
    {
        xSemaphoreGive(xParameterMutex);
        return xError;
    }

    xPersistedParameters = xFactoryParameters;
    xFactoryParameters.u8FunctionFlags = UART_PROTOCOL_FLAG_FACTORY_RESET;    
    xError = UartManagerWriteParameters(&xFactoryParameters); // Registra o comando antes de liberar o mutex
    xSemaphoreGive(xParameterMutex);

    return xError;
}

/*********************************************************************************************************
* @brief    Função que envia um comando de função ao escravo
* @fn       esp_err_t StorageManagerSendCommand(uint8_t u8FunctionFlags)
* @param    u8FunctionFlags      @brief ponteiro para os parametros da tarefa
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerSendCommand(uint8_t u8FunctionFlags)
{
    DEVICE_PARAMETERS_t xTransmitParameters;
    esp_err_t xError;

    if (u8FunctionFlags != UART_PROTOCOL_FLAG_NETWORK_LEAVE)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xParameterMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    xTransmitParameters = xDeviceParameters;
    xTransmitParameters.u8FunctionFlags = u8FunctionFlags;
    xError = UartManagerWriteParameters(&xTransmitParameters);
    xSemaphoreGive(xParameterMutex);

    return xError;
}

/*********************************************************************************************************
* @brief    Função que sincroniza os parametros do mestre com o escravo
* @fn       esp_err_t StorageManagerSyncWithSlave(void)
* @param    void      @brief void
* @return   esp_err_t         @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t StorageManagerSyncWithSlave(void)
{
    DEVICE_PARAMETERS_t xParameters;
    esp_err_t xError;

    if (xParameterMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(xParameterMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    xParameters = xDeviceParameters;
    xError = UartManagerWriteParameters(&xParameters);
    xSemaphoreGive(xParameterMutex);

    return xError;
}
