/*
 * HwInit.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef _HWINIT_H_
#define _HWINIT_H_
// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"

// Exported constants ----------------------------------------------------------
#define BUTTON_LEDS_ON_DUTY_CYCLE           32000
#define BUTTON_LEDS_NIGHT_MODE_DUTY_CYCLE   16000

#define LED_SLIDER_0_PORT                   GPIO2
#define LED_SLIDER_0_PIN                    GPIO_PIN_0
#define LED_SLIDER_0_OFF                    GPIO_WriteHigh(LED_SLIDER_0_PORT, LED_SLIDER_0_PIN)
#define LED_SLIDER_0_ON                     GPIO_WriteLow(LED_SLIDER_0_PORT, LED_SLIDER_0_PIN)
#define LED_SLIDER_0_TOGGLE                 GPIO_TogglePin(LED_SLIDER_0_PORT,LED_SLIDER_0_PIN)

#define LED_SLIDER_1_PORT                   GPIO2
#define LED_SLIDER_1_PIN                    GPIO_PIN_1
#define LED_SLIDER_1_OFF                    GPIO_WriteHigh(LED_SLIDER_1_PORT, LED_SLIDER_1_PIN)
#define LED_SLIDER_1_ON                     GPIO_WriteLow(LED_SLIDER_1_PORT, LED_SLIDER_1_PIN)
#define LED_SLIDER_1_TOGGLE                 GPIO_TogglePin(LED_SLIDER_1_PORT, LED_SLIDER_1_PIN)
    
#define LED_SLIDER_2_PORT                   GPIO2
#define LED_SLIDER_2_PIN                    GPIO_PIN_2
#define LED_SLIDER_2_OFF                    GPIO_WriteHigh(LED_SLIDER_2_PORT, LED_SLIDER_2_PIN)
#define LED_SLIDER_2_ON                     GPIO_WriteLow(LED_SLIDER_2_PORT, LED_SLIDER_2_PIN)
#define LED_SLIDER_2_TOGGLE                 GPIO_TogglePin(LED_SLIDER_2_PORT, LED_SLIDER_2_PIN)

#define LED_SLIDER_3_PORT                   GPIO2
#define LED_SLIDER_3_PIN                    GPIO_PIN_3
#define LED_SLIDER_3_OFF                    GPIO_WriteHigh(LED_SLIDER_3_PORT, LED_SLIDER_3_PIN)
#define LED_SLIDER_3_ON                     GPIO_WriteLow(LED_SLIDER_3_PORT, LED_SLIDER_3_PIN)
#define LED_SLIDER_3_TOGGLE                 GPIO_TogglePin(LED_SLIDER_3_PORT, LED_SLIDER_3_PIN)

#define LED_SLIDER_4_PORT                   GPIO2
#define LED_SLIDER_4_PIN                    GPIO_PIN_4
#define LED_SLIDER_4_OFF                    GPIO_WriteHigh(LED_SLIDER_4_PORT, LED_SLIDER_4_PIN)
#define LED_SLIDER_4_ON                     GPIO_WriteLow(LED_SLIDER_4_PORT, LED_SLIDER_4_PIN)
#define LED_SLIDER_4_TOGGLE                 GPIO_TogglePin(LED_SLIDER_4_PORT, LED_SLIDER_4_PIN)

#define LED_SLIDER_5_PORT                   GPIO2
#define LED_SLIDER_5_PIN                    GPIO_PIN_5
#define LED_SLIDER_5_OFF                    GPIO_WriteHigh(LED_SLIDER_5_PORT, LED_SLIDER_5_PIN)
#define LED_SLIDER_5_ON                     GPIO_WriteLow(LED_SLIDER_5_PORT, LED_SLIDER_5_PIN)
#define LED_SLIDER_5_TOGGLE                 GPIO_TogglePin(LED_SLIDER_5_PORT, LED_SLIDER_5_PIN)

#define LED_SLIDER_6_PORT                   GPIO1
#define LED_SLIDER_6_PIN                    GPIO_PIN_6
#define LED_SLIDER_6_OFF                    GPIO_WriteHigh(LED_SLIDER_6_PORT, LED_SLIDER_6_PIN)
#define LED_SLIDER_6_ON                     GPIO_WriteLow(LED_SLIDER_6_PORT, LED_SLIDER_6_PIN)
#define LED_SLIDER_6_TOGGLE                 GPIO_TogglePin(LED_SLIDER_6_PORT, LED_SLIDER_6_PIN)

#define LED_SLIDER_7_PORT                   GPIO1
#define LED_SLIDER_7_PIN                    GPIO_PIN_7
#define LED_SLIDER_7_OFF                    GPIO_WriteHigh(LED_SLIDER_7_PORT, LED_SLIDER_7_PIN)
#define LED_SLIDER_7_ON                     GPIO_WriteLow(LED_SLIDER_7_PORT, LED_SLIDER_7_PIN)
#define LED_SLIDER_7_TOGGLE                 GPIO_TogglePin(LED_SLIDER_7_PORT, LED_SLIDER_7_PIN)

#define LED_SLIDER_8_PORT                   GPIO2
#define LED_SLIDER_8_PIN                    GPIO_PIN_6
#define LED_SLIDER_8_OFF                    GPIO_WriteHigh(LED_SLIDER_8_PORT, LED_SLIDER_8_PIN)
#define LED_SLIDER_8_ON                     GPIO_WriteLow(LED_SLIDER_8_PORT, LED_SLIDER_8_PIN)
#define LED_SLIDER_8_TOGGLE                 GPIO_TogglePin(LED_SLIDER_8_PORT, LED_SLIDER_8_PIN)

#define LED_SLIDER_9_PORT                   GPIO2
#define LED_SLIDER_9_PIN                    GPIO_PIN_7
#define LED_SLIDER_9_OFF                    GPIO_WriteHigh(LED_SLIDER_9_PORT, LED_SLIDER_9_PIN)
#define LED_SLIDER_9_ON                     GPIO_WriteLow(LED_SLIDER_9_PORT, LED_SLIDER_9_PIN)
#define LED_SLIDER_9_TOGGLE                 GPIO_TogglePin(LED_SLIDER_9_PORT, LED_SLIDER_9_PIN)

#define LED_SLIDER_10_PORT                  GPIO1
#define LED_SLIDER_10_PIN                   GPIO_PIN_1
#define LED_SLIDER_10_OFF                   GPIO_WriteHigh(LED_SLIDER_10_PORT, LED_SLIDER_10_PIN)
#define LED_SLIDER_10_ON                    GPIO_WriteLow(LED_SLIDER_10_PORT, LED_SLIDER_10_PIN)
#define LED_SLIDER_10_TOGGLE                GPIO_TogglePin(LED_SLIDER_10_PORT, LED_SLIDER_10_PIN)

#define BUTTON_PORT                         GPIO0
#define BUTTON_PIN                          GPIO_PIN_7
#define BUTTON_STATUS                       GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN)

#define BUTTON_LEDS_MAX_COUNT_VALUE         31999
#define BUTTON_LEDS_HALF_COUNT_VALUE        (BUTTON_LEDS_MAX_COUNT_VALUE / 2)
#define BUTTON_LEDS_PORT                    GPIO0
#define BUTTON_LEDS_PIN                     GPIO_PIN_6
#define BUTTON_LEDS_OFF                     PWM_IndependentModeConfig(PWM21, BUTTON_LEDS_MAX_COUNT_VALUE)
#define BUTTON_LEDS_ON                      PWM_IndependentModeConfig(PWM21, 0)
#define BUTTON_LEDS_IN_NIGHT_MODE           PWM_IndependentModeConfig(PWM21, BUTTON_LEDS_HALF_COUNT_VALUE)
#define BUTTON_LEDS_SET_COUNTER(x)          PWM_IndependentModeConfig(PWM21, x);

#define SLIDER_OFF                          (LED_SLIDER_0_OFF, LED_SLIDER_1_OFF, LED_SLIDER_2_OFF, LED_SLIDER_3_OFF, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_0                      (LED_SLIDER_0_ON, LED_SLIDER_1_OFF, LED_SLIDER_2_OFF, LED_SLIDER_3_OFF, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_1                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_OFF, LED_SLIDER_3_OFF, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_2                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_OFF, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_3                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_4                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_OFF, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_5                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_6                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_ON, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_7                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_ON, LED_SLIDER_7_ON, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_8                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_ON, LED_SLIDER_7_ON, LED_SLIDER_8_ON, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_9                      (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_ON, LED_SLIDER_7_ON, LED_SLIDER_8_ON, LED_SLIDER_9_ON, LED_SLIDER_10_OFF)
#define SLIDER_LEVEL_10                     (LED_SLIDER_0_ON, LED_SLIDER_1_ON, LED_SLIDER_2_ON, LED_SLIDER_3_ON, LED_SLIDER_4_ON, LED_SLIDER_5_ON, LED_SLIDER_6_ON, LED_SLIDER_7_ON, LED_SLIDER_8_ON, LED_SLIDER_9_ON, LED_SLIDER_10_ON)
#define SLIDER_TOGGLE                       (LED_SLIDER_0_TOGGLE, LED_SLIDER_1_TOGGLE, LED_SLIDER_2_TOGGLE, LED_SLIDER_3_TOGGLE, LED_SLIDER_4_TOGGLE, LED_SLIDER_5_TOGGLE, LED_SLIDER_6_TOGGLE, LED_SLIDER_7_TOGGLE, LED_SLIDER_8_TOGGLE, LED_SLIDER_9_TOGGLE, LED_SLIDER_10_TOGGLE)
#define SLIDER_TOGGLE_FOR_LEADING_MODE      (LED_SLIDER_0_TOGGLE, LED_SLIDER_1_TOGGLE, LED_SLIDER_2_TOGGLE, LED_SLIDER_3_TOGGLE, LED_SLIDER_4_TOGGLE, LED_SLIDER_5_TOGGLE, LED_SLIDER_6_OFF, LED_SLIDER_7_OFF, LED_SLIDER_8_OFF, LED_SLIDER_9_OFF, LED_SLIDER_10_OFF)
#define SLIDER_TOGGLE_FOR_TRAILING_MODE     (LED_SLIDER_0_OFF, LED_SLIDER_1_OFF, LED_SLIDER_2_OFF, LED_SLIDER_3_OFF, LED_SLIDER_4_OFF, LED_SLIDER_5_OFF, LED_SLIDER_6_TOGGLE, LED_SLIDER_7_TOGGLE, LED_SLIDER_8_TOGGLE, LED_SLIDER_9_TOGGLE, LED_SLIDER_10_TOGGLE)

#define TOUCH_PAD0_PORT                     GPIO0
#define TOUCH_PAD0_PIN                      GPIO_PIN_0

#define TOUCH_PAD1_PORT                     GPIO0
#define TOUCH_PAD1_PIN                      GPIO_PIN_1

#define TOUCH_PAD2_PORT                     GPIO0
#define TOUCH_PAD2_PIN                      GPIO_PIN_2

#define TOUCH_PAD3_PORT                     GPIO0
#define TOUCH_PAD3_PIN                      GPIO_PIN_3

#define TOUCH_PAD4_PORT                     GPIO0
#define TOUCH_PAD4_PIN                      GPIO_PIN_4

#define TOUCH_PAD5_PORT                     GPIO0
#define TOUCH_PAD5_PIN                      GPIO_PIN_5

#define UART_TX_PORT                        GPIO1
#define UART_TX_PIN                         GPIO_PIN_4

#define UART_RX_PORT                        GPIO1
#define UART_RX_PIN                         GPIO_PIN_5

// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
void HwInit(void);
    
#endif
