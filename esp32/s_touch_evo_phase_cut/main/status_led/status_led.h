/*
 *  status_led.h
 *
 *  Created on: Nov 03, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef _STATUS_LED_H
#define _STATUS_LED_H

// Includes --------------------------------------------------------------------
#include "esp_err.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef enum
{
    STATUS_LED_MODE_NORMAL = 0,
    STATUS_LED_SLAVE_COMM_ERROR,
    STATUS_LED_TEMPERATURE_ERROR 
} STATUS_LED_MODE_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
esp_err_t StatusLedInit(void);
void StatusLedSetMode(STATUS_LED_MODE_t eMode);


// Exported functions ----------------------------------------------------------
esp_err_t StatusLedInit(void);
void StatusLedSetMode(STATUS_LED_MODE_t eMode);


#endif