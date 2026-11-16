/*
 *  hardware_config.h
 *
 *  Created on: Nov 03, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// Includes --------------------------------------------------------------------
#include "driver/gpio.h"
#include "driver/uart.h"

// Exported constants ----------------------------------------------------------
#define STATUS_LED_GPIO                     GPIO_NUM_23
#define STATUS_LED_ACTIVE_LEVEL             1
#define STATUS_LED_INACTIVE_LEVEL           0

#define SLAVE_UART_PORT                     UART_NUM_1
#define SLAVE_UART_TX_GPIO                  GPIO_NUM_0
#define SLAVE_UART_RX_GPIO                  GPIO_NUM_1
#define SLAVE_UART_BAUDRATE                 9600
#define SLAVE_UART_RX_BUFFER_SIZE           256
#define SLAVE_UART_TX_BUFFER_SIZE           0

#define NTC_GPIO                            GPIO_NUM_3
#define NTC_ADC_GPIO                        GPIO_NUM_3
#define NTC_ADC_UNIT                        ADC_UNIT_1
#define NTC_ADC_CHANNEL                     ADC_CHANNEL_3
#define NTC_ADC_ATTENUATION                 ADC_ATTEN_DB_12
#define NTC_ADC_BITWIDTH                    ADC_BITWIDTH_12

#define ZERO_CROSSING_GPIO                  GPIO_NUM_18
#define ZERO_CROSSING_INTR_TYPE             GPIO_INTR_ANYEDGE

#define MOSFET_TRIGGER_GPIO                 GPIO_NUM_6
#define MOSFET_TRIGGER_ACTIVE_LEVEL         1
#define MOSFET_TRIGGER_INACTIVE_LEVEL       0

#define INTERNAL_TEMP_RANGE_MIN_C           20
#define INTERNAL_TEMP_RANGE_MAX_C           100

// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------

#endif
