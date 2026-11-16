/*
* Comm.c
*
*  Created on: Oct 28, 2026
*      Author: Rodrigo Arnhold
*/

// Includes --------------------------------------------------------------------
#include "Comm.h"
#include "Utils.h"

// Private define --------------------------------------------------------------
#define UART_PROTOCOL_START_BYTE                                0xAA

#define UART_PROTOCOL_START_MARKER_POSITION                     0
#define UART_PROTOCOL_COMMAND_POSITION                          1

#define UART_PROTOCOL_READ_REQUEST_SIZE                         4
#define UART_PROTOCOL_FRAME_SIZE                                12

#define UART_PROTOCOL_UART_RX_TIMEOUT_TICKS                     2 // duas interrupções de BTM (15,625ms) = 31,25ms 

#define UART_PROTOCOL_CRC_POLYNOMIAL                            0x07
#define UART_PROTOCOL_CRC_INITIAL                               0x00
#define UART_PROTOCOL_CRC_XOR_OUT                               0x00

#define UART_PROTOCOL_FLAG_NETWORK_LEAVE                        (1 << 0)
#define UART_PROTOCOL_FLAG_FACTORY_RESET                        (1 << 1)
#define UART_PROTOCOL_FLAG_NETWORK_LEAVE_CONFIRMATION           (1 << 2)
#define UART_PROTOCOL_FLAG_FACTORY_RESET_CONFIRMATION           (1 << 3)
#define UART_PROTOCOL_MASTER_COMMAND_MASK                       0x03

#define DIM_LEVEL_MIN                           0
#define DIM_LEVEL_MAX                           100

// Private typedef -------------------------------------------------------------
typedef enum
{
    UART_PROTOCOL_IDX_START = 0,
    UART_PROTOCOL_IDX_COMMAND,
    UART_PROTOCOL_IDX_SEQUENCE,
    UART_PROTOCOL_IDX_FUNCTION_FLAGS,
    UART_PROTOCOL_IDX_DEVICE_OUTPUT_STATE,
    UART_PROTOCOL_IDX_DIM_LEVEL,
    UART_PROTOCOL_IDX_MIN_DIM_LEVEL,
    UART_PROTOCOL_IDX_MAX_DIM_LEVEL,
    UART_PROTOCOL_IDX_NIGHT_MODE_STATUS,
    UART_PROTOCOL_IDX_LOCK_MODE_STATUS,
    UART_PROTOCOL_IDX_PHASE_CUT_MODE,
    UART_PROTOCOL_IDX_CRC
} UART_PROTOCOL_IDX_t;

typedef enum
{
    STATE_RECEPTION_IDLE = 0,
    STATE_RECEPTION_IN_PROGRESS,
    STATE_RECEPTION_FINISHED
} UART_PROTOCOL_RECEPTION_STATES_t;

typedef enum
{
    UART_PROTOCOL_READ_COMMAND = 0x00,
    UART_PROTOCOL_WRITE_COMMAND
} UART_PROTOCOL_MESSAGE_TYPE_t;

typedef struct
{
    volatile uint8_t u8BufferCounter;
    volatile uint8_t u8Buffer[UART_PROTOCOL_FRAME_SIZE];
    volatile uint16_t u16ReceptionTimer;
    volatile UART_PROTOCOL_RECEPTION_STATES_t eReceptionState;
} UART_PROTOCOL_RECEPTION_t;

// Private variables -----------------------------------------------------------
static UART_PROTOCOL_RECEPTION_t xUartProtocolReceptionParams;
static USCI2_HandleInfoDef xUsci2Handle;
static uint8_t u8TxBuffer[UART_PROTOCOL_FRAME_SIZE];
static UART_PROTOCOL_MESSAGE_TYPE_t eMessageCommand;
static bool bResponsePending;

// Private function prototypes -------------------------------------------------
static uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, uint8_t u8Len);
static void UartProtocolReceiveMessage(uint8_t u8Byte);
static void UartProtocolCheckReceptionTimeout(void);
static bool UartProtocolValidateParameters(const DEVICE_PARAMETERS_t *pxParameters);
static void UartProtocolBuildResponse(uint8_t u8Command, uint8_t u8Sequence, const DEVICE_PARAMETERS_t *pxParameters, uint8_t u8FunctionFlags);
static void UartProtocolResetReception(void);
static void UartProtocolTransmitPendingResponse(void);

// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função que calcula o CRC8 dos bytes das mensagens
/// @fn		static uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, uint8_t u8Len)
/// @Param	*pu8Data        @brief ponteiro para o buffer de dados
/// @Param  u8Len           @brief numero de bytes do buffer
/// @return uint8_t         @brief Valor calculado para o crc8
// *****************************************************************************
static uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, uint8_t u8Len)
{
    uint8_t u8Crc = UART_PROTOCOL_CRC_INITIAL;
    uint8_t i;
    uint8_t j;

    for (i = 0; i < u8Len; i++)
    {
        u8Crc ^= pu8Data[i];

        for (j = 0; j < 8; j++)
        {
            if (u8Crc & 0x80)
            {
                u8Crc = (u8Crc << 1) ^ UART_PROTOCOL_CRC_POLYNOMIAL;
            }
            else
            {
                u8Crc <<= 1;
            }
        }
    }

    return u8Crc ^ UART_PROTOCOL_CRC_XOR_OUT;
}

// *****************************************************************************
/// @brief	Função que retransmite uma resposta pendente
/// @fn		static void UartProtocolTransmitPendingResponse(void)
/// @Param	void       @brief void
/// @return void       @brief Valor calculado para o crc8
// *****************************************************************************
static void UartProtocolTransmitPendingResponse(void)
{
    StatusTypeDef eStatus;

    if (bResponsePending == FALSE)
    {
        return;
    }

    if (xUsci2Handle.TxState != USCI2_STATE_READY)
    {
        return;
    }

    eStatus = USCI2_UART_Transmit_IT(&xUsci2Handle, u8TxBuffer, UART_PROTOCOL_FRAME_SIZE);

    if (eStatus == Status_OK)
    {
        bResponsePending = FALSE;
        UartProtocolResetReception();
    }
}

// *****************************************************************************
/// @brief	Função que monitora o timout da recepção
/// @fn		static void UartProtocolCheckReceptionTimeout(void)
/// @Param	void     @brief void
/// @return void     @brief void
// *****************************************************************************
static void UartProtocolCheckReceptionTimeout(void)
{
    uint16_t u16CurrentTime;

    if (xUartProtocolReceptionParams.eReceptionState != STATE_RECEPTION_IN_PROGRESS)
    {
        return;
    }

    u16CurrentTime = ReadSysTime();

    if ((uint16_t)(u16CurrentTime -xUartProtocolReceptionParams.u16ReceptionTimer) >= UART_PROTOCOL_UART_RX_TIMEOUT_TICKS)
    {
        xUartProtocolReceptionParams.u8BufferCounter = 0;
        xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_IDLE;
    }
}

// *****************************************************************************
/// @brief	Função de inicialização dos parametros de comunicação
/// @fn		void UartProtocolInit(void)
/// @Param	void     @brief void
/// @return void     @brief void
// *****************************************************************************
void UartProtocolInit(void)
{
    uint8_t i;

    bResponsePending = FALSE;
    xUartProtocolReceptionParams.u8BufferCounter = 0;
    xUartProtocolReceptionParams.u16ReceptionTimer = ReadSysTime();
    xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_IDLE;

    for (i = 0; i < UART_PROTOCOL_FRAME_SIZE; i++)
    {
        xUartProtocolReceptionParams.u8Buffer[i] = 0;
        u8TxBuffer[i] = 0;
    }

    xUsci2Handle.TxState = USCI2_STATE_READY;
    xUsci2Handle.TxXferSize = 0;
    xUsci2Handle.TxXferCount = 0;
    
    USCI2_ClearFlag(USCI2_UART_FLAG_RI);
    USCI2_ClearFlag(USCI2_UART_FLAG_TI);
}

// *****************************************************************************
/// @brief	Função que recebe um byte de dados
/// @fn		void UartProtocolReceiveByte(uint8_t u8Byte)
/// @Param	u8Byte     @brief byte de dados recebido
/// @return void       @brief void
// *****************************************************************************
void UartProtocolReceiveByte(uint8_t u8Byte)
{
    UartProtocolReceiveMessage(u8Byte);
}

// *****************************************************************************
/// @brief	Função que recebe dados da inteface serial e monta a mensagem byte a byte
/// @fn		static void UartProtocolReceiveMessage(uint8_t u8Byte)
/// @Param  u8Byte      @brief byte de dados recebido
/// @return void        @brief void
// *****************************************************************************
static void UartProtocolReceiveMessage(uint8_t u8Byte)
{
    uint8_t i;

    // Durante a transmissao da resposta, bytes recebidos sao descartados.
    if (xUsci2Handle.TxState == USCI2_STATE_BUSY)
    {
        return;
    }

    if (xUartProtocolReceptionParams.eReceptionState == STATE_RECEPTION_IDLE)
    {
        if (u8Byte == UART_PROTOCOL_START_BYTE)
        {
            xUartProtocolReceptionParams.u16ReceptionTimer = ReadSysTime();
            xUartProtocolReceptionParams.u8BufferCounter = 0;

            for (i = 0; i < UART_PROTOCOL_FRAME_SIZE; i++)
            {
                xUartProtocolReceptionParams.u8Buffer[i] = 0;
            }

            xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_START_MARKER_POSITION] = u8Byte;
            xUartProtocolReceptionParams.u8BufferCounter = 1;
            xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_IN_PROGRESS;
        }
    }
    else if (xUartProtocolReceptionParams.eReceptionState == STATE_RECEPTION_IN_PROGRESS)
    {
        if (xUartProtocolReceptionParams.u8BufferCounter == UART_PROTOCOL_COMMAND_POSITION)
        {
            if ((UART_PROTOCOL_MESSAGE_TYPE_t)u8Byte == UART_PROTOCOL_READ_COMMAND)
            {
                eMessageCommand = UART_PROTOCOL_READ_COMMAND;
            }
            else if ((UART_PROTOCOL_MESSAGE_TYPE_t)u8Byte == UART_PROTOCOL_WRITE_COMMAND)
            {
                eMessageCommand = UART_PROTOCOL_WRITE_COMMAND;
            }
            else
            {
                UartProtocolResetReception();
                return;
            }
        }
        
        if (eMessageCommand == UART_PROTOCOL_READ_COMMAND)
        {
            if (xUartProtocolReceptionParams.u8BufferCounter < UART_PROTOCOL_READ_REQUEST_SIZE)
            {
                xUartProtocolReceptionParams.u8Buffer[xUartProtocolReceptionParams.u8BufferCounter] = u8Byte;
                xUartProtocolReceptionParams.u8BufferCounter++;
                xUartProtocolReceptionParams.u16ReceptionTimer = ReadSysTime();

                if (xUartProtocolReceptionParams.u8BufferCounter == UART_PROTOCOL_READ_REQUEST_SIZE)
                {
                    xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_FINISHED;
                }
            }
            else
            {
                UartProtocolResetReception();
            }            
        }
        else if (eMessageCommand == UART_PROTOCOL_WRITE_COMMAND)
        {
            if (xUartProtocolReceptionParams.u8BufferCounter < UART_PROTOCOL_FRAME_SIZE)
            {
                xUartProtocolReceptionParams.u8Buffer[xUartProtocolReceptionParams.u8BufferCounter] = u8Byte;
                xUartProtocolReceptionParams.u8BufferCounter++;
                xUartProtocolReceptionParams.u16ReceptionTimer = ReadSysTime();

                if (xUartProtocolReceptionParams.u8BufferCounter == UART_PROTOCOL_FRAME_SIZE)
                {
                    xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_FINISHED;
                }
            }
            else
            {
                UartProtocolResetReception();
            }             
        }
        else
        {
            UartProtocolResetReception();
            return;            
        }
    }
}

// *****************************************************************************
/// @brief	Tarefa que processa a mensagem de resposta
/// @fn	    static bool UartProtocolValidateParameters(const DEVICE_PARAMETERS_t *pxParameters)
/// @Param	*pxParameters   @brief ponteiro para os parametros recebidos do mestre
/// @return bool         @brief true se os dados esteverem corretos, false se não
// *****************************************************************************
static bool UartProtocolValidateParameters(const DEVICE_PARAMETERS_t *pxParameters)
{
    if (pxParameters->u8FunctionFlags & (uint8_t)(~UART_PROTOCOL_MASTER_COMMAND_MASK))
    {
        return FALSE;
    }
    
    if (pxParameters->u8DeviceOutputState > OUTPUT_ON)
    {
        return FALSE;
    }

    if (pxParameters->u8DimLevel > DIM_LEVEL_MAX)
    {
        return FALSE;
    }

    if (pxParameters->u8MinDimLevel > DIM_LEVEL_MAX)
    {
        return FALSE;
    }

    if (pxParameters->u8MaxDimLevel > DIM_LEVEL_MAX)
    {
        return FALSE;
    }

    if (pxParameters->u8MinDimLevel > pxParameters->u8MaxDimLevel)
    {
        return FALSE;
    }

    if (pxParameters->u8NightModeStatus > NIGHT_MODE_ENABLE)
    {
        return FALSE;
    }

    if (pxParameters->u8LockModeStatus > LOCK_MODE_ENABLE)
    {
        return FALSE;
    }
     
    if (pxParameters->u8PhaseCutMode > PHASE_CUT_MODE_TRAILING)
    {
        return FALSE;
    }    
   
    return TRUE;
}

// *****************************************************************************
/// @brief	Função que monta a resposta do escravo
/// @fn	    static void UartProtocolBuildResponse(uint8_t u8Command, uint8_t u8Sequence, const DEVICE_PARAMETERS_t *pxParameters, uint8_t u8FunctionFlags)
/// @Param	u8Command       @brief comando recebido read ou write
/// @Param	u8Sequence      @brief contador sequencial da mensagem
/// @Param	*pxParameters   @brief ponteiro para a estrutura de parametros do dispositivo
/// @Param	u8FunctionFlags @brief variável que contém as flags de funções (factory reset, network leave)
/// @return void           @brief void
// *****************************************************************************
static void UartProtocolBuildResponse(uint8_t u8Command, uint8_t u8Sequence, const DEVICE_PARAMETERS_t *pxParameters, uint8_t u8FunctionFlags)
{
    u8TxBuffer[UART_PROTOCOL_IDX_START]               = UART_PROTOCOL_START_BYTE;
    u8TxBuffer[UART_PROTOCOL_IDX_COMMAND]             = u8Command;
    u8TxBuffer[UART_PROTOCOL_IDX_SEQUENCE]            = u8Sequence;
    u8TxBuffer[UART_PROTOCOL_IDX_FUNCTION_FLAGS]      = u8FunctionFlags;
    u8TxBuffer[UART_PROTOCOL_IDX_DEVICE_OUTPUT_STATE] = pxParameters->u8DeviceOutputState;
    u8TxBuffer[UART_PROTOCOL_IDX_DIM_LEVEL]           = pxParameters->u8DimLevel;
    u8TxBuffer[UART_PROTOCOL_IDX_MIN_DIM_LEVEL]       = pxParameters->u8MinDimLevel;
    u8TxBuffer[UART_PROTOCOL_IDX_MAX_DIM_LEVEL]       = pxParameters->u8MaxDimLevel;
    u8TxBuffer[UART_PROTOCOL_IDX_NIGHT_MODE_STATUS]   = pxParameters->u8NightModeStatus;
    u8TxBuffer[UART_PROTOCOL_IDX_LOCK_MODE_STATUS]    = pxParameters->u8LockModeStatus;
    u8TxBuffer[UART_PROTOCOL_IDX_PHASE_CUT_MODE]      = pxParameters->u8PhaseCutMode;
    u8TxBuffer[UART_PROTOCOL_IDX_CRC]                 = UartProtocolCalculateCrc8((const uint8_t *)u8TxBuffer, (UART_PROTOCOL_FRAME_SIZE - 1));
}

// *****************************************************************************
/// @brief	Função que reinicializa a maquina de estados de recepção
/// @fn	    static void UartProtocolResetReception(void)
/// @Param	void   @brief void
/// @return void   @brief void
// *****************************************************************************
static void UartProtocolResetReception(void)
{
    xUartProtocolReceptionParams.u8BufferCounter = 0;
    xUartProtocolReceptionParams.eReceptionState = STATE_RECEPTION_IDLE;
}

// *****************************************************************************
/// @brief	Tarefa que processa a mensagem de resposta
/// @fn	    void UartProtocolResponseProcess(DEVICE_PARAMETERS_t *xDevParams)
/// @Param	xDevParams   @brief Ponteiro para a estrutura de parametros do dispositivo
/// @return void         @brief void
// *****************************************************************************
void UartProtocolResponseProcess(DEVICE_PARAMETERS_t *xDevParams)
{   
    UartProtocolCheckReceptionTimeout();       
    
    if (bResponsePending == TRUE) // Se existe uma resposta pendente, tenta transmiti-la e não processa novamente a mensagem recebida
    {
        UartProtocolTransmitPendingResponse();
        return;
    }
    
    if (xUsci2Handle.TxState != USCI2_STATE_READY)
    {
        return;
    }

    if (xUartProtocolReceptionParams.eReceptionState != STATE_RECEPTION_FINISHED)
    {
        return;
    }
    
    if (xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_COMMAND_POSITION] == UART_PROTOCOL_READ_COMMAND)
    {
        uint8_t u8CalculatedCrc = UartProtocolCalculateCrc8((const uint8_t *)xUartProtocolReceptionParams.u8Buffer, (UART_PROTOCOL_READ_REQUEST_SIZE - 1));

        if (xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_READ_REQUEST_SIZE - 1] != u8CalculatedCrc)
        {
            UartProtocolResetReception();
            return;
        }
        
        UartProtocolBuildResponse(UART_PROTOCOL_READ_COMMAND, xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_SEQUENCE], xDevParams, xDevParams->u8FunctionFlags);
        bResponsePending = TRUE;
        UartProtocolTransmitPendingResponse();
    }   
    else if (xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_COMMAND_POSITION] == UART_PROTOCOL_WRITE_COMMAND)
    {
        DEVICE_PARAMETERS_t xTempDevParams;
        uint8_t u8ReceivedFlags = 0;
        uint8_t u8ExecutedFlags = 0;
        uint8_t u8CalculatedCrc = UartProtocolCalculateCrc8((const uint8_t *)xUartProtocolReceptionParams.u8Buffer, (UART_PROTOCOL_FRAME_SIZE - 1));

        if (xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_CRC] != u8CalculatedCrc)
        {
            UartProtocolResetReception();
            return;
        }
        
        u8ReceivedFlags = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_FUNCTION_FLAGS];           
            
        xTempDevParams.u8FunctionFlags     = u8ReceivedFlags;
        xTempDevParams.u8DeviceOutputState = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_DEVICE_OUTPUT_STATE];
        xTempDevParams.u8DimLevel          = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_DIM_LEVEL];
        xTempDevParams.u8MinDimLevel       = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_MIN_DIM_LEVEL]; 
        xTempDevParams.u8MaxDimLevel       = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_MAX_DIM_LEVEL];
        xTempDevParams.u8NightModeStatus   = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_NIGHT_MODE_STATUS];
        xTempDevParams.u8LockModeStatus    = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_LOCK_MODE_STATUS];
        xTempDevParams.u8PhaseCutMode      = xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_PHASE_CUT_MODE];
    
        if (UartProtocolValidateParameters(&xTempDevParams) == FALSE)
        {
            UartProtocolResetReception();
            return;
        }
        
        if (u8ReceivedFlags & UART_PROTOCOL_FLAG_NETWORK_LEAVE)
        {
            ExecuteNetworkLeave();
            u8ExecutedFlags |= UART_PROTOCOL_FLAG_NETWORK_LEAVE_CONFIRMATION;
        }

        if (u8ReceivedFlags & UART_PROTOCOL_FLAG_FACTORY_RESET)
        {
            ExecuteFactoryReset();
            u8ExecutedFlags |= UART_PROTOCOL_FLAG_FACTORY_RESET_CONFIRMATION;
        }
        
        xDevParams->u8FunctionFlags    &= (uint8_t)(~u8ReceivedFlags);
        xDevParams->u8DeviceOutputState = xTempDevParams.u8DeviceOutputState;
        xDevParams->u8DimLevel          = xTempDevParams.u8DimLevel;
        xDevParams->u8MinDimLevel       = xTempDevParams.u8MinDimLevel;
        xDevParams->u8MaxDimLevel       = xTempDevParams.u8MaxDimLevel;
        xDevParams->u8NightModeStatus   = xTempDevParams.u8NightModeStatus;
        xDevParams->u8LockModeStatus    = xTempDevParams.u8LockModeStatus;
        xDevParams->u8PhaseCutMode      = xTempDevParams.u8PhaseCutMode;
        
        UartProtocolBuildResponse(UART_PROTOCOL_WRITE_COMMAND, xUartProtocolReceptionParams.u8Buffer[UART_PROTOCOL_IDX_SEQUENCE], xDevParams, u8ExecutedFlags);
        bResponsePending = TRUE;
        UartProtocolTransmitPendingResponse(); 
    }
    else
    {
        return;
    }
}

// *****************************************************************************
/// @brief	Função que envia a mensagem de resposta
/// @fn		void UartProtocolTransmit_IRQHandler(void)
/// @Param	void     @brief void
/// @return void     @brief void
// *****************************************************************************
void UartProtocolTransmit_IRQHandler(void)
{
    if (xUsci2Handle.TxState == USCI2_STATE_BUSY)
    {
        (void)USCI2_UART_Transmit_IRQHandler(&xUsci2Handle);
    }
}
