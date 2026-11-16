/*
 * Interrupts.c
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

// Includes --------------------------------------------------------------------
#include "Interrupts.h"
#include "Utils.h"
#include "HwInit.h"
#include "Comm.h"

// Private define --------------------------------------------------------------
// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
// Private function prototypes -------------------------------------------------
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função de tratamento a interrupção externa 0
/// @fn		void INT0Interrupt()
/// @Param	None                  @brief  None
/// @return None                  @brief  None
// *****************************************************************************
void INT0Interrupt()		interrupt 0				
{
    TCON &= 0XFD;
}

// *****************************************************************************
/// @brief	Função de tratamento a interrupção externa 1
/// @fn		void INT1Interrupt()
/// @Param	None                  @brief  None
/// @return None                  @brief  None
// *****************************************************************************
void INT1Interrupt()		interrupt 2		
{
    TCON &= 0XF7;			
}

// *****************************************************************************
/// @brief	Função de tratamento a interrupção do timer BTM
/// @fn		void BTMInterrupt()
/// @Param	None                  @brief  None
/// @return None                  @brief  None
// *****************************************************************************
void BTMInterrupt()			interrupt 9
{
	IncBTM_Counter();
    BTM_ClearFlag();
}

// *****************************************************************************
/// @brief	Função de tratamento a interrupçao da porta USCI2
/// @fn		void USCI2Interrupt()
/// @Param	None                  @brief  None
/// @return None                  @brief  None
// *****************************************************************************
void USCI2Interrupt() interrupt 16
{
    if (USCI2_GetFlagStatus(USCI2_UART_FLAG_RI) == SET)
    {
        uint8_t u8Byte = USCI2_UART_ReceiveData8();
        USCI2_ClearFlag(USCI2_UART_FLAG_RI);
        UartProtocolReceiveByte(u8Byte);
    }

    if (USCI2_GetFlagStatus(USCI2_UART_FLAG_TI) == SET)
    {
        USCI2_ClearFlag(USCI2_UART_FLAG_TI);
        UartProtocolTransmit_IRQHandler();
    }
}
