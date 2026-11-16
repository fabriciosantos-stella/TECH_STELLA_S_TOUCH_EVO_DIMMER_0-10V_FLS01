/*
 *  uart_manager.h
 *
 *  Created on: Nov 03, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef UART_MANAGER_H
#define UART_MANAGER_H

// Includes --------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "uart_protocol.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t UartManagerInit(void);
esp_err_t UartManagerStart(void);
esp_err_t UartManagerWriteParameters(const DEVICE_PARAMETERS_t *pxParameters);
esp_err_t UartManagerReadParameters(void);
bool UartManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters);
bool UartManagerIsCommunicationOk(void);
bool UartManagerAreParametersSynchronized(void);
esp_err_t UartManagerGetPendingSlaveRequests(uint8_t *pu8FunctionFlags);
bool UartManagerCanAcceptSlaveParameters(void);

#endif
