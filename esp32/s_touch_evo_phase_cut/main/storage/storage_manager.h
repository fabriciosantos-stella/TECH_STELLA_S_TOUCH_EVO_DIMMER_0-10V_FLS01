/*
 *  parameter_manager.h
 *
 *  Created on: Nov 03, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef PARAMETER_MANAGER_H
#define PARAMETER_MANAGER_H

// Includes --------------------------------------------------------------------
#include <stdint.h>
#include "esp_err.h"
#include "slider_interface/uart_protocol.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t StorageManagerInit(void);
esp_err_t StorageManagerGetParameters(DEVICE_PARAMETERS_t *pxParameters);
esp_err_t StorageManagerSetParameters(const DEVICE_PARAMETERS_t *pxParameters);
esp_err_t StorageManagerFactoryReset(void);
esp_err_t StorageManagerSendCommand(uint8_t u8FunctionFlags);
esp_err_t StorageManagerSyncWithSlave(void);

#endif