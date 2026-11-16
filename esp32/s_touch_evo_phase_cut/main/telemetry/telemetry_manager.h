/*
 *  telemetry_manager.h
 *
 *  Created on: Nov 12, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef TELEMETRY_MANAGER_H
#define TELEMETRY_MANAGER_H

// Includes --------------------------------------------------------------------
#include <stdint.h>
#include "esp_err.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef struct
{
    float fMosfetTemperature;
    float fMcuTemperature;
    float fLineFrequency;
} TELEMETRY_DATA_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t TelemetryManagerInit(void);
void TelemetryManagerUpdateTemperatures(float fMosfetTemperature, float fMcuTemperature);
void TelemetryManagerUpdateFrequency(float fLineFrequency);
esp_err_t TelemetryManagerGetData(TELEMETRY_DATA_t *pxData);

#endif


/*********** EXEMPLO DE IMPLEMENTAÇÃO DE CONSULTA DE VALORES DE TELEMETRIA *************    

TELEMETRY_DATA_t xTelemetry;

if (TelemetryManagerGetData(&xTelemetry) == ESP_OK)
{
    ESP_LOGI(TAG, "MOSFET: %.2f C", xTelemetry.fMosfetTemperature);
    ESP_LOGI(TAG, "MCU: %.2f C", xTelemetry.fMcuTemperature);
    ESP_LOGI(TAG, "Mains: %.2f Hz", xTelemetry.fLineFrequency);
}

****************************************************************************************/