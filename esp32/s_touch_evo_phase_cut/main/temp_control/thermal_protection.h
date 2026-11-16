/*
 *  thermal_protection.h
 *
 *  Created on: Nov 12, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef THERMAL_PROTECTION_H
#define THERMAL_PROTECTION_H

// Includes --------------------------------------------------------------------
#include "esp_err.h"
#include <stdbool.h>

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef enum
{
    THERMAL_PROTECTION_INITIALIZING = 0,
    THERMAL_PROTECTION_NORMAL,
    THERMAL_PROTECTION_OVER_TEMPERATURE,
    THERMAL_PROTECTION_SENSOR_FAULT
} THERMAL_PROTECTION_STATE_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t ThermalProtectionInit(void);
esp_err_t ThermalProtectionGetState(THERMAL_PROTECTION_STATE_t *peState);
esp_err_t ThermalProtectionIsOutputAllowed(bool *pbAllowed);

#endif
