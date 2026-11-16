/*
 *  uart_protocol.h
 *
 *  Created on: Nov 03, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef _UART_PROTOCOL_H
#define _UART_PROTOCOL_H

// Includes --------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Exported constants ----------------------------------------------------------
#define UART_PROTOCOL_START_BYTE                        0xAA
#define UART_PROTOCOL_READ_COMMAND                      0x00
#define UART_PROTOCOL_WRITE_COMMAND                     0x01

#define UART_PROTOCOL_READ_REQUEST_SIZE                 4
#define UART_PROTOCOL_WRITE_REQUEST_SIZE                12
#define UART_PROTOCOL_READ_RESPONSE_SIZE                12
#define UART_PROTOCOL_WRITE_RESPONSE_SIZE               12

#define UART_PROTOCOL_PARAMETERS_SIZE                   8

#define UART_PROTOCOL_READ_REQUEST_CRC_INDEX            3
#define UART_PROTOCOL_FRAME_CRC_INDEX                   11

#define UART_PROTOCOL_FLAG_NETWORK_LEAVE                0x01
#define UART_PROTOCOL_FLAG_FACTORY_RESET                0x02
#define UART_PROTOCOL_FLAG_NETWORK_LEAVE_CONFIRMATION   0x04
#define UART_PROTOCOL_FLAG_FACTORY_RESET_CONFIRMATION   0x08
#define UART_PROTOCOL_MASTER_COMMAND_MASK               0x03
#define UART_PROTOCOL_SLAVE_CONFIRMATION_MASK           0x0C

// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
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
} UART_PROTOCOL_INDEX_t;

typedef struct
{
    uint8_t u8FunctionFlags;
    uint8_t u8DeviceOutputState;
    uint8_t u8DimLevel;
    uint8_t u8MinDimLevel;
    uint8_t u8MaxDimLevel;
    uint8_t u8NightModeStatus;
    uint8_t u8LockModeStatus;
    uint8_t u8PhaseCutMode;
} DEVICE_PARAMETERS_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
uint8_t UartProtocolCalculateCrc8(const uint8_t *pu8Data, size_t xLength);
bool UartProtocolBuildReadRequest(uint8_t u8Sequence, uint8_t *pu8Buffer, size_t xBufferSize);
bool UartProtocolBuildWriteRequest(uint8_t u8Sequence, const DEVICE_PARAMETERS_t *pxParameters, uint8_t *pu8Buffer, size_t xBufferSize);
bool UartProtocolValidateFrame(const uint8_t *pu8Buffer, size_t xLength, uint8_t u8ExpectedCommand, uint8_t u8ExpectedSequence);
bool UartProtocolParseParameters(const uint8_t *pu8Buffer, size_t xLength, DEVICE_PARAMETERS_t *pxParameters);

#endif