/*
 * Slider.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Rodrigo Arnhold
 */
 
#ifndef _SLIDER_H_
#define	_SLIDER_H_
// Includes --------------------------------------------------------------------
#include "sc95f_conf.h"

// Exported constants ----------------------------------------------------------
// Exported enumerations -------------------------------------------------------
enum
{
    SLIDER_LEVEL_ZERO = 0,
    SLIDER_LEVEL_ONE_VALUE,
    SLIDER_LEVEL_TWO_VALUE,
    SLIDER_LEVEL_THREE_VALUE,
    SLIDER_LEVEL_FOUR_VALUE,
    SLIDER_LEVEL_FIVE_VALUE,
    SLIDER_LEVEL_SIX_VALUE,
    SLIDER_LEVEL_SEVEN_VALUE,
    SLIDER_LEVEL_EIGHT_VALUE,
    SLIDER_LEVEL_NINE_VALUE,
    SLIDER_LEVEL_TEN_VALUE,
    SLIDER_LEVEL_ELEVEN_VALUE
};

// Exported types --------------------------------------------------------------
typedef enum
{
    SLIDER_MODE_NORMAL = 0,
    SLIDER_MODE_CONFIG_MIN_DIM_LIMIT,
    SLIDER_MODE_CONFIG_MAX_DIM_LIMIT
} SLIDER_MODE_t;

// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
void SliderRead(SLIDER_MODE_t eMode, uint8_t* u8DimLevel);

#endif