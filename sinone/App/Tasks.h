/*
 * Tasks.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */
 
#ifndef _TASKS_H_
#define	_TASKS_H_

// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
enum PhaseCutModes
{
    PHASE_CUT_MODE_LEADING = 0,
    PHASE_CUT_MODE_TRAILING
};

enum DeviceOutputStates
{
    OUTPUT_OFF = 0,
    OUTPUT_ON
};

enum NightModeStates
{
    NIGHT_MODE_DISABLE = 0,
    NIGHT_MODE_ENABLE
};

enum LockModeStates
{
    LOCK_MODE_DISABLE = 0,
    LOCK_MODE_ENABLE
};

enum DeviceFunctionFlags
{
    NETWORK_LEAVE_FLAG,                     // flag que sinaliza solicitação de leave da rede zigbee
    FACTORY_RESET_FLAG                      // flag que sinaliza solicitação de reset de fabrica.
};

// Exported types --------------------------------------------------------------
typedef struct
{
    uint8_t u8FunctionFlags;
    uint8_t u8DeviceOutputState;
    uint8_t u8DimLevel;
    uint8_t u8MinDimLevel;
    uint8_t u8MaxDimLevel;
    uint8_t u8NightModeStatus;
    uint8_t u8LockModeStatus;
    uint8_t u8PhaseCutMode;
} DEVICE_PARAMETERS_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
void TasksInit(void);
void TaskComm(void);
void TaskSwitchScan(void);
void TaskConfigHandle(void);
void ExecuteNetworkLeave(void);
void ExecuteFactoryReset(void);

#endif
