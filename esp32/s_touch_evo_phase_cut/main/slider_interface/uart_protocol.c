/*
 *  uart_protocol.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "uart_protocol.h"
#include <string.h>

// Private define ----------------------------------------------------------------------------------------
#define UART_PROTOCOL_CRC_POLYNOMIAL        0x07
#define UART_PROTOCOL_CRC_INITIAL           0x00
#define UART_PROTOCOL_CRC_XOR_OUT           0x00

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
// Private function prototypes ---------------------------------------------------------------------------
// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Função que calcula o crc8 de um pacote de bytes
* @fn       uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, size_t xLength)
* @param    *pu8Data    @brief ponteiro para o vetor de bytes da mensagem
* @param    xLength     @brief numero de bytes da mensagem
* @return   uint8_t     @brief retorna o crc calculado
*********************************************************************************************************/
uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, size_t xLength)
{
    uint8_t u8Crc = UART_PROTOCOL_CRC_INITIAL;

    if (pu8Data == NULL)
    {
        return 0;
    }

    for (size_t xIndex = 0; xIndex < xLength; xIndex++)
    {
        u8Crc ^= pu8Data[xIndex];

        for (uint8_t u8Bit = 0; u8Bit < 8; u8Bit++)
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

/*********************************************************************************************************
* @brief    Função que calcula o crc8 de um pacote de bytes
* @fn       uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, size_t xLength)
* @param    u8Sequence      @brief contador sequencial da mensagem
* @param    *pu8Buffer      @brief ponteiro para o buffer de transmissão
* @param    xBufferSize     @brief numero de bytes do buffer de dados
* @return   bool            @brief true para sucesso, false para falha
*********************************************************************************************************/
bool UartProtocolBuildReadRequest(uint8_t u8Sequence, uint8_t *pu8Buffer, size_t xBufferSize)
{
    if ((pu8Buffer == NULL) || (xBufferSize < UART_PROTOCOL_READ_REQUEST_SIZE))
    {
        return false;
    }

    pu8Buffer[UART_PROTOCOL_IDX_START] = UART_PROTOCOL_START_BYTE;
    pu8Buffer[UART_PROTOCOL_IDX_COMMAND] = UART_PROTOCOL_READ_COMMAND;
    pu8Buffer[UART_PROTOCOL_IDX_SEQUENCE] = u8Sequence;
    pu8Buffer[UART_PROTOCOL_READ_REQUEST_CRC_INDEX] = UartProtocolCalculateCrc8(pu8Buffer, UART_PROTOCOL_READ_REQUEST_CRC_INDEX);

    return true;
}

/*********************************************************************************************************
* @brief    Função que calcula o crc8 de um pacote de bytes
* @fn       uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, size_t xLength)
* @param    u8Sequence      @brief número sequencial da mensagem
* @param    *pxParameters   @brief ponteiro para a estrutura de parametros
* @param    *puBuffer       @brief ponteiro para o buffer de transmissão
* @param    xBufferSize     @brief numero de bytes do buffer de dados
* @return   bool            @brief true para sucesso, false para falha
*********************************************************************************************************/
bool UartProtocolBuildWriteRequest(uint8_t u8Sequence, const DEVICE_PARAMETERS_t *pxParameters, uint8_t *pu8Buffer, size_t xBufferSize)
{
    if ((pu8Buffer == NULL) || (pxParameters == NULL) || (xBufferSize < UART_PROTOCOL_WRITE_REQUEST_SIZE))
    {
        return false;
    }

    pu8Buffer[UART_PROTOCOL_IDX_START]                  = UART_PROTOCOL_START_BYTE;
    pu8Buffer[UART_PROTOCOL_IDX_COMMAND]                = UART_PROTOCOL_WRITE_COMMAND;
    pu8Buffer[UART_PROTOCOL_IDX_SEQUENCE]               = u8Sequence;
    pu8Buffer[UART_PROTOCOL_IDX_FUNCTION_FLAGS]         = pxParameters->u8FunctionFlags;
    pu8Buffer[UART_PROTOCOL_IDX_DEVICE_OUTPUT_STATE]    = pxParameters->u8DeviceOutputState;
    pu8Buffer[UART_PROTOCOL_IDX_DIM_LEVEL]              = pxParameters->u8DimLevel;
    pu8Buffer[UART_PROTOCOL_IDX_MIN_DIM_LEVEL]          = pxParameters->u8MinDimLevel;
    pu8Buffer[UART_PROTOCOL_IDX_MAX_DIM_LEVEL]          = pxParameters->u8MaxDimLevel;
    pu8Buffer[UART_PROTOCOL_IDX_NIGHT_MODE_STATUS]      = pxParameters->u8NightModeStatus;
    pu8Buffer[UART_PROTOCOL_IDX_LOCK_MODE_STATUS]       = pxParameters->u8LockModeStatus;
    pu8Buffer[UART_PROTOCOL_IDX_PHASE_CUT_MODE]         = pxParameters->u8PhaseCutMode;
    pu8Buffer[UART_PROTOCOL_IDX_CRC]                    = UartProtocolCalculateCrc8(pu8Buffer, UART_PROTOCOL_FRAME_CRC_INDEX);

    return true;
}

/*********************************************************************************************************
* @brief    Função que valida a mensagem recebida
* @fn       bool UartProtocolValidateFrame(const uint8_t *pu8Buffer, size_t xLength, uint8_t u8ExpectedCommand, uint8_t u8ExpectedSequence)
* @param    *pu8Buffer          @brief ponteiro para o vetor de bytes da mensagem
* @param    xLength             @brief número de bytes da mensagem 
* @param    u8ExpectedCommand   @brief código do comando esperado
* @param    u8ExpectedSequence  @brief valor de sequência esperado
* @return   bool                @brief true se os dados forem validos, false se não forem
*********************************************************************************************************/
bool UartProtocolValidateFrame(const uint8_t *pu8Buffer, size_t xLength, uint8_t u8ExpectedCommand, uint8_t u8ExpectedSequence)
{
    if (pu8Buffer == NULL)
    {
        return false;
    }

    if ((u8ExpectedCommand != UART_PROTOCOL_READ_COMMAND) && (u8ExpectedCommand != UART_PROTOCOL_WRITE_COMMAND))
    {
        return false;
    }

    if (u8ExpectedCommand == UART_PROTOCOL_READ_COMMAND)
    {
        if (xLength != UART_PROTOCOL_READ_RESPONSE_SIZE)
        {
            return false;
        }
    }
    else
    {
        if (xLength != UART_PROTOCOL_WRITE_RESPONSE_SIZE)
        {
            return false;
        }
    }

    if (pu8Buffer[UART_PROTOCOL_IDX_START] != UART_PROTOCOL_START_BYTE)
    {
        return false;
    }

    if (pu8Buffer[UART_PROTOCOL_IDX_COMMAND] != u8ExpectedCommand)
    {
        return false;
    }

    if (pu8Buffer[UART_PROTOCOL_IDX_SEQUENCE] != u8ExpectedSequence)
    {
        return false;
    }

    if (UartProtocolCalculateCrc8(pu8Buffer, UART_PROTOCOL_FRAME_CRC_INDEX) != pu8Buffer[UART_PROTOCOL_IDX_CRC])
    {
        return false;
    }

    return true;
}

/*********************************************************************************************************
* @brief    Função que faz o parser da mensagem recebida e copia os dados para a estrutura de parametros
* @fn       bool UartProtocolParseParameters(const uint8_t *pu8Buffer, size_t xLength, DEVICE_PARAMETERS_t *pxParameters)
* @param    *pu8Buffer          @brief ponteiro para o vetor de bytes da mensagem
* @param    xLength             @brief número de bytes da mensagem 
* @param    *pxParameters       @brief ponteiro para a estrutura de parametros
* @return   bool                @brief true se o pacote foi parseado false se houver falha
*********************************************************************************************************/
bool UartProtocolParseParameters(const uint8_t *pu8Buffer, size_t xLength, DEVICE_PARAMETERS_t *pxParameters)
{
    if ((pu8Buffer == NULL) || (pxParameters == NULL))
    {
        return false;
    }

    if ((xLength != UART_PROTOCOL_READ_RESPONSE_SIZE) && (xLength != UART_PROTOCOL_WRITE_RESPONSE_SIZE))
    {
        return false;
    }

    if ((pu8Buffer[UART_PROTOCOL_IDX_COMMAND] != UART_PROTOCOL_READ_COMMAND) && (pu8Buffer[UART_PROTOCOL_IDX_COMMAND] != UART_PROTOCOL_WRITE_COMMAND))
    {
        return false;
    }

    if (!UartProtocolValidateFrame(pu8Buffer, xLength, pu8Buffer[UART_PROTOCOL_IDX_COMMAND], pu8Buffer[UART_PROTOCOL_IDX_SEQUENCE]))
    {
        return false;
    }

    pxParameters->u8FunctionFlags       = pu8Buffer[UART_PROTOCOL_IDX_FUNCTION_FLAGS];
    pxParameters->u8DeviceOutputState   = pu8Buffer[UART_PROTOCOL_IDX_DEVICE_OUTPUT_STATE];
    pxParameters->u8DimLevel            = pu8Buffer[UART_PROTOCOL_IDX_DIM_LEVEL];
    pxParameters->u8MinDimLevel         = pu8Buffer[UART_PROTOCOL_IDX_MIN_DIM_LEVEL];
    pxParameters->u8MaxDimLevel         = pu8Buffer[UART_PROTOCOL_IDX_MAX_DIM_LEVEL];
    pxParameters->u8NightModeStatus     = pu8Buffer[UART_PROTOCOL_IDX_NIGHT_MODE_STATUS];
    pxParameters->u8LockModeStatus      = pu8Buffer[UART_PROTOCOL_IDX_LOCK_MODE_STATUS];
    pxParameters->u8PhaseCutMode        = pu8Buffer[UART_PROTOCOL_IDX_PHASE_CUT_MODE];

    return true;
}
