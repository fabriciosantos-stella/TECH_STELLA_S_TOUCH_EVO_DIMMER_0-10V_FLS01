/*
* Utils.c
*
*  Created on: Jun 10, 2026
*      Author: Rodrigo Arnhold
*/

// Includes --------------------------------------------------------------------
#include "Utils.h"

// Private define --------------------------------------------------------------
// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
volatile uint16_t u16BtmTickCounter = 0;

// Private function prototypes -------------------------------------------------
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função que atualiza o contador de 15,625ms do BTM
/// @fn		void IncBTM_Counter(void)
/// @Param	none
/// @return none
// *****************************************************************************
void IncBTM_Counter(void)
{
    u16BtmTickCounter++;
}

// *****************************************************************************
/// @brief	Função que retorna o valor do contador da base de tempo do sistema
/// @fn		uint16_t ReadSysTime(void)
/// @Param	None          @brief None
/// @return uint16_t      @brief Valor do contador da base de tempo selecionada
// *****************************************************************************
uint16_t ReadSysTime(void)
{
	uint16_t u16Temp = 0;

	do
	{
		u16Temp = u16BtmTickCounter;
	}
	while (u16Temp != u16BtmTickCounter);

	return u16Temp;
}

// *****************************************************************************
/// @brief	Função que retorna o valor do contador da base de tempo do sistema
/// @fn		uint16_t TimeDiff(eSYS_TIME_Refs_t eRef, uint16_t u16OldTime)
/// @Param	u16OldTime          @brief The previous conter value
/// @return u16ElapsedTime      @brief The elapsed time
// *****************************************************************************
uint16_t TimeDiff(uint16_t u16OldTime)
{
	uint16_t u16Now = 0;
	uint16_t u16ElapsedTime = 0;

	u16Now = ReadSysTime();
	u16ElapsedTime = u16Now >= u16OldTime ? (u16Now - u16OldTime) : (0xFFFF - u16OldTime + u16Now);

	return u16ElapsedTime;
}

// *****************************************************************************
/// @brief	Redirecionamento de printf para porta uart0
/// @fn		char putchar(char c)
/// @Param	c           @brief Byte a ser enviado
/// @return None        @brief None
// *****************************************************************************
char putchar(char c)
{
    UART0_SendData8(c);
    while(!UART0_GetFlagStatus(UART0_FLAG_TI));
    UART0_ClearFlag(UART0_FLAG_TI);
    
    return c;
}

// *****************************************************************************
/// @brief	Função que escreve um byte na serial
/// @fn		void WriteSerial(uint8_t u8Byte)
/// @Param	u8Byte      @brief Byte a ser enviado
/// @return None        @brief None
// *****************************************************************************
void WriteSerial(uint8_t u8Byte)
{
    UART0_SendData8(u8Byte);
    while(!UART0_GetFlagStatus(UART0_FLAG_TI));
    UART0_ClearFlag(UART0_FLAG_TI);  
}
