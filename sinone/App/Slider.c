/*
 * Slider.c
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */

// Includes --------------------------------------------------------------------
#include "Slider.h"
#include "TKDriver.h"
#include "Utils.h"
#include "HwInit.h"
#include "Tasks.h"

// Private define --------------------------------------------------------------
#define SLIDER_SCAN_END_BIT_MASK    0x80
#define SLIDER_SCAN_ALL_BITS_MASK   0x7F

// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
// Private function prototypes -------------------------------------------------
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Funcao que trata a posição atualizada do slider
/// @fn		static void SliderLedsUpdate(uint8_t u8Value, SLIDER_MODE_t eMode)
/// @Param	uint8_t     @brief  Valor atual do slider
/// @Param  eMode       @brief  modo de sinalização dos led do slider (normal ou em modo ajuste)
/// @return None        @brief  None
// *****************************************************************************
static void SliderLedsUpdate(uint8_t u8Value, SLIDER_MODE_t eMode)
{
    switch (u8Value)
    {
        case SLIDER_LEVEL_ONE_VALUE:
        {
            if (eMode == SLIDER_MODE_NORMAL)
            {
                SLIDER_LEVEL_0;                
            }
            else
            {
                LED_SLIDER_0_OFF;
            }
            break;
        }            
        case SLIDER_LEVEL_TWO_VALUE:
        {
            if (eMode == SLIDER_MODE_NORMAL)
            {
                SLIDER_LEVEL_1;
            }
            else if (eMode == SLIDER_MODE_CONFIG_MAX_DIM_LIMIT)
            {
                SLIDER_LEVEL_1;
            }
            break;
        }
        case SLIDER_LEVEL_THREE_VALUE:
        {
            SLIDER_LEVEL_2;
            break;
        }
        case SLIDER_LEVEL_FOUR_VALUE:
        {
            SLIDER_LEVEL_3;
            break;
        }
        case SLIDER_LEVEL_FIVE_VALUE:
        {
            SLIDER_LEVEL_4;
            break;
        }
        case SLIDER_LEVEL_SIX_VALUE:
        {
            SLIDER_LEVEL_5;
            break;
        }        
        case SLIDER_LEVEL_SEVEN_VALUE:
        {
            SLIDER_LEVEL_6;
            break;
        }
        case SLIDER_LEVEL_EIGHT_VALUE:
        {
            SLIDER_LEVEL_7;
            break;
        }
        case SLIDER_LEVEL_NINE_VALUE:
        {
            SLIDER_LEVEL_8;
            break;
        }
        case SLIDER_LEVEL_TEN_VALUE:
        {
            SLIDER_LEVEL_9;
            break;
        }  
        case SLIDER_LEVEL_ELEVEN_VALUE:
        {
            if (eMode == SLIDER_MODE_NORMAL) 
            {
                SLIDER_LEVEL_10;
            }
            else if (eMode == SLIDER_MODE_CONFIG_MIN_DIM_LIMIT)
            {
                SLIDER_LEVEL_10;
            }
            break;
        } 
    }
}

// *****************************************************************************
/// @brief	Funcao que faz o processamento do valor do slider
/// @fn		static void SliderDataProcessing(unsigned int u16Value, uint8_t* u8DimLevel)
/// @Param	u16Value    @brief  nivel de dimerização
/// @Param	u8DimLevel  @brief  ponteiro para o parametro da estrutura de parametro do dispositivo
/// @return void        @brief  void
// *****************************************************************************
static void SliderDataProcessing(unsigned int u16Value, uint8_t* u8DimLevel)
{
    static uint16_t u16SliderPreviousValue = 0xFFFF;
    
    if (u16Value > 0)
    {
        if (u16Value != u16SliderPreviousValue)
        {
            u16SliderPreviousValue = u16Value;
            *u8DimLevel = (uint8_t)u16Value;         
        }
    }   
}        

// *****************************************************************************
/// @brief	Funcão que faz a leitura da posição do slider
/// @fn		void SliderRead(SLIDER_MODE_t eMode, uint8_t* u8DimLevel)
/// @Param  eMode       @brief  modo de sinalização dos led do slider (normal ou em modo ajuste)
/// @Param	u8DimLevel  @brief  ponteiro para o parametro da estrutura de parametro do dispositivo
/// @return void        @brief  void
// *****************************************************************************
void SliderRead(SLIDER_MODE_t eMode, uint8_t* u8DimLevel)
{
    if (SOCAPI_TouchKeyStatus & SLIDER_SCAN_END_BIT_MASK) // Processa o valor do slider se todas as posições foram lidas
    {
        SOCAPI_TouchKeyStatus &= SLIDER_SCAN_ALL_BITS_MASK; // Limpa o bit do ciclo completo para a proxima varredura
        TouchKeyScan(); 
        SliderDataProcessing(TKSlideModulePCBArray[0].OutValue, u8DimLevel); 
        TouchKeyRestart();      
    }
    
    SliderLedsUpdate(*u8DimLevel, eMode);
}
