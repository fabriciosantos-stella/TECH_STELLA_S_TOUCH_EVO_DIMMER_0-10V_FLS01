/*
 * Firmware_Version.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */
 
#ifndef _FIRMWARE_VERSION_H_
#define	_FIRMWARE_VERSION_H_
// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"

// Exported constants ----------------------------------------------------------
#define FW_VERSION_MAJOR 0
#define FW_VERSION_MINOR 0
#define FW_VERSION_PATCH 1
#define FW_VERSION_STAGE "alpha"  // alpha, beta, rc, release

#define FW_VERSION_STRING "0.0.1-alpha"
#define FW_BUILD_DATE __DATE__
#define FW_BUILD_TIME __TIME__

// Exported types --------------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------

#endif // _FIRMWARE_VERSION_H_