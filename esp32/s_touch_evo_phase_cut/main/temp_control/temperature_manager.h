/*
 *  temperature_manager.h
 *
 *  Created on: Nov 12, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

// Includes --------------------------------------------------------------------
#include "esp_err.h"
#include <stdbool.h>

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef struct
{
    int iNtcRawValue;
    float fNtcTemperature;
    float fInternalTemperature;
    bool bNtcValid;
    bool bInternalTemperatureValid;
} TEMPERATURE_DATA_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t TemperatureManagerInit(void);
esp_err_t TemperatureManagerGetData(TEMPERATURE_DATA_t *pxData);

#endif
