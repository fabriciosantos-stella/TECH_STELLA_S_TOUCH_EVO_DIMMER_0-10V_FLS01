/*
 * HwInit.c
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

// Includes --------------------------------------------------------------------
#include "HwInit.h"

// Private define --------------------------------------------------------------
#define PWM_PSC     31999
#define SYS_CLK     32000000
#define UART0_BAUD  9600
#define UART2_BAUD  9600

// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
// Private function prototypes -------------------------------------------------
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função de configuração dos clocks e parametros basicos do sistema
/// @fn		void OPTION_Config(void)
/// @Param	None                  @brief  None
/// @return None                  @brief  None
// *****************************************************************************
void OPTION_Config(void)
{
	SC95F8573_NIO_Init();
	OPTION_SYSCLK_Init(SYSCLK_PRESSEL_FOSC_D1);
	OPTION_LVR_Init(LVR_1_9V);
	OPTION_IAP_SetOperateRange(IAP_OPERATERANGE__LAST_0K_CODEREGION);
	OPTION_RST_PIN_Cmd(DISABLE);
	OPTION_XTIPLL_Cmd(DISABLE);
	OPTION_JTG_Cmd(ENABLE);
}

// *****************************************************************************
/// @brief	Função de inicialização dos GPIOs
/// @fn		void GPIO_Config(void)
/// @Param	none
/// @return none
// *****************************************************************************
void GPIO_Config(void)
{
	GPIO_Init(LED_SLIDER_10_PORT, LED_SLIDER_10_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_6_PORT, LED_SLIDER_6_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_7_PORT, LED_SLIDER_7_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_9_PORT, LED_SLIDER_9_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_8_PORT, LED_SLIDER_8_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_0_PORT, LED_SLIDER_0_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_1_PORT, LED_SLIDER_1_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_2_PORT, LED_SLIDER_2_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_3_PORT, LED_SLIDER_3_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_4_PORT, LED_SLIDER_4_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(LED_SLIDER_5_PORT, LED_SLIDER_5_PIN, GPIO_MODE_OUT_PP);
	GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_PU);
	GPIO_Init(TOUCH_PAD0_PORT, TOUCH_PAD0_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(TOUCH_PAD1_PORT, TOUCH_PAD1_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(TOUCH_PAD2_PORT, TOUCH_PAD2_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(TOUCH_PAD3_PORT, TOUCH_PAD3_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(TOUCH_PAD4_PORT, TOUCH_PAD4_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(TOUCH_PAD5_PORT, TOUCH_PAD5_PIN, GPIO_MODE_OUT_PP);
    GPIO_Init(UART_TX_PORT, UART_TX_PIN, GPIO_MODE_IN_PU);
	GPIO_Init(UART_RX_PORT, UART_RX_PIN, GPIO_MODE_IN_PU);
}

// *****************************************************************************
/// @brief	Função de inicialização das portas PWM
/// @fn		void PWM_Config(void)
/// @Param	none
/// @return none
// *****************************************************************************
void PWM_Config(void)
{
    PWM_Init(PWM2_PRESSEL_FSYS_D1, PWM_PSC);
    PWM_IndependentModeConfigEX(PWM21, 0, PWM_OUTPUTSTATE_ENABLE);
    PWM_ITConfigEX(PWM2_Type, DISABLE, LOW);
    PWM_CmdEX(PWM2_Type, ENABLE);
}

// *****************************************************************************
/// @brief	Função de inicialização da UART2
/// @fn		void UART2_Config(void)
/// @Param	none
/// @return none
// *****************************************************************************
void UART2_Config(void)
{
  	USCI2_UART_Init(SYS_CLK, UART2_BAUD, USCI2_UART_Mode_10B, USCI2_UART_RX_ENABLE);  
    USCI2_ClearFlag(USCI2_UART_FLAG_RI);
    USCI2_ClearFlag(USCI2_UART_FLAG_TI);
    USCI2_ITConfig(ENABLE, LOW);
}

// *****************************************************************************
/// @brief	Função de inicialização do watchdog
/// @fn		void WDT_Config(void)
/// @Param	none
/// @return none
// *****************************************************************************
void WDT_Config(void)
{
    WDT_Init(WDT_OverflowTime_500MS);
	OPTION_WDT_Cmd(ENABLE);	
}

// *****************************************************************************
/// @brief	Função de inicialização do timer BTM
/// @fn		void BTM_Config(void)
/// @Param	none
/// @return none
// *****************************************************************************
void BTM_Config(void)
{
    BTM_Init(BTM_TIMEBASE_15_625MS);
    BTM_Cmd(ENABLE);    
    BTM_ITConfig(ENABLE, LOW);
}

// *****************************************************************************
/// @brief	Função de inicialização do MCU
/// @fn		void HwInit(void)
/// @Param	none
/// @return none
// *****************************************************************************
void HwInit(void)
{    
    GPIO_Config();
    OPTION_Config();
    PWM_Config();    
    UART2_Config();
    WDT_Config();
    BTM_Config();
}
