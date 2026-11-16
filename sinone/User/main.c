/*
 * main.c
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

// Includes --------------------------------------------------------------------
#include "main.h"
#include "HwInit.h"	
#include "Firmware_Version.h"
#include "TKDriver.h"
#include "Comm.h"
#include "Tasks.h"

// Private define --------------------------------------------------------------
// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
// Private function prototypes -------------------------------------------------
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função principal do sistema
/// @fn		void main(void)
/// @Param	none
/// @return none
// *****************************************************************************
void main(void)
{	
    HwInit();
    TouchKeyInit();  
    TasksInit();
    UartProtocolInit();
    
    EA = 1;
    
    while(1)
    {      
        TaskComm();
        TaskSwitchScan();
        TaskConfigHandle();
        WDT_SetReload();
    }
}