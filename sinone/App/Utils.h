/*
 * Utils.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

#ifndef _UTILS_H_
#define _UTILS_H_
// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
void IncBTM_Counter(void);
uint16_t ReadSysTime(void);
uint16_t TimeDiff(uint16_t u16OldTime);
void WriteSerial(uint8_t u8Byte);

#endif