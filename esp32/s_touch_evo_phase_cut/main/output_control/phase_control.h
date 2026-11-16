/*
 *  phase_control.h
 *
 *  Created on: Nov 10, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef PHASE_CONTROL_H
#define PHASE_CONTROL_H

// Includes --------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef struct
{
    uint32_t u32HalfCycleTimeUs;
    float fLineFrequencyHz;
    bool bFrequencyValid;
} PHASE_CONTROL_DATA_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t PhaseControlInit(void);
esp_err_t PhaseControlGetData(PHASE_CONTROL_DATA_t *pxData);

#endif