/*
 * Comm.h
 *
 *  Created on: Oct 28, 2026
 *      Author: Rodrigo Arnhold
 */
 
#ifndef _COMM_H_
#define	_COMM_H_

// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"
#include "Tasks.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
void UartProtocolInit(void);
void UartProtocolResponseProcess(DEVICE_PARAMETERS_t *xDevParams);
void UartProtocolReceiveByte(uint8_t u8Byte);
void UartProtocolTransmit_IRQHandler(void);

#endif