/*
 *  output_control.h
 *
 *  Created on: Nov 17, 2026
 *      Author: fabricio.santos
 */

#ifndef OUTPUT_CONTROL_H
#define OUTPUT_CONTROL_H

#include <stdint.h>
#include "esp_err.h"

esp_err_t OutputControlInit(void);
esp_err_t OutputControlSetLevel(uint8_t u8Level);      // 0-11, aplica imediato
void OutputControlSetPercent(uint8_t u8Percent);       // 0-100, aplica imediato
uint8_t OutputControlGetCurrentPercent(void);          // retorna percentual atual 0-100
void OutputControlSetSK1(uint8_t u8State);
void OutputControlSetDimmerMode(uint8_t u8State);

#endif /* OUTPUT_CONTROL_H */
