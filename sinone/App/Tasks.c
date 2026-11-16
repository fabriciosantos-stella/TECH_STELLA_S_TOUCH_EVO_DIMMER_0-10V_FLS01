/*
* StateMachine.c
*
*  Created on: Jun 10, 2026
*      Author: Rodrigo Arnhold
*/

// Includes --------------------------------------------------------------------
#include "Tasks.h"
#include "HwInit.h"
#include "Utils.h"
#include "Comm.h"
#include <string.h>
#include "TKDriver.h"
#include "Slider.h"

// Private define --------------------------------------------------------------
#define SWITCH_HOLD_TIMEOUT                                     320 // igual a 5000ms, pois as interrupções do btm ocorrem a cada 15,625ms
#define SWITCH_RELEASED_TIMOUT                                  32  // igual a 500ms

#define SWITCH_PRESS_TIMES_TO_ZIGBEE_LEAVE                      2
#define SWITCH_PRESS_TIMES_TO_SET_PHASE_CUT_MODE                3
#define SWITCH_PRESS_TIMES_TO_SET_NIGHT_MODE                    4
#define SWITCH_PRESS_TIMES_TO_SET_DIM_LIMITS                    5
#define SWITCH_PRESS_TIMES_TO_SET_LOCK_MODE                     6
#define SWITCH_PRESS_TIMES_TO_FACTORY_RESET                     7

#define DEFAULT_LED_BLINK_DELAY                                 32 // 500ms
#define ZIGBEE_PAIRING_DEFECT_DELAY                             6  // 93,75ms

#define LOCK_MODE_DISABLE_BLINK_TIMES                           6
#define LOCK_MODE_ENABLE_BLINK_TIMES                            4 
#define PHASE_CUT_CHANGE_BLINK_TIMES                            8
#define FACTORY_RESET_BLINK_TIMES                               8

// Private typedef -------------------------------------------------------------
typedef enum
{
    CONFIG_FEEDBACK_IDLE = 0,
    CONFIG_FEEDBACK_PHASE_CUT_MODE,
    CONFIG_FEEDBACK_DIM_LIMITS,
    CONFIG_FEEDBACK_LOCK_MODE,
    CONFIG_FEEDBACK_NIGHT_MODE,
    CONFIG_FEEDBACK_FACTORY_RESET,
    CONFIG_FEEDBACK_ZIGBEE_LEAVE
} CONFIG_FEEDBACK_STATE_t;

typedef enum 
{
    DIM_LIMITS_ADJUST_NONE = 0,
    DIM_LIMITS_ADJUST_ADD,
    DIM_LIMITS_ADJUST_MIN,
    DIM_LIMITS_ADJUST_MAX
} MIN_DIM_ADJUST_STATE_t;

// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
DEVICE_PARAMETERS_t xDeviceParams;
CONFIG_FEEDBACK_STATE_t eConfigFeedbackState;
MIN_DIM_ADJUST_STATE_t eMinDimAjustState;
SLIDER_MODE_t eSliderMode;
uint16_t u16ButtomPwmDuty;

// Private function prototypes -------------------------------------------------
static void TaskSliderScan(SLIDER_MODE_t eMode);
static void TaskSwitchLeds(void);
    
// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Função de inicialização dos parametros do dispositivo
/// @fn		void TasksInit(void)
/// @Param	void     @brief void
/// @return void     @brief void
// *****************************************************************************
void TasksInit(void)
{
    eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
    eMinDimAjustState = DIM_LIMITS_ADJUST_NONE;
    eSliderMode = SLIDER_MODE_NORMAL;    
    xDeviceParams.u8FunctionFlags = 0;
    xDeviceParams.u8DeviceOutputState = 0;
    xDeviceParams.u8DimLevel = 0; 
    xDeviceParams.u8MinDimLevel = 0;
    xDeviceParams.u8MaxDimLevel = 0;
    xDeviceParams.u8NightModeStatus = 0;
    xDeviceParams.u8LockModeStatus = 0;
    xDeviceParams.u8PhaseCutMode = 0;
}

// *****************************************************************************
/// @brief	Tarefa que trata a leitura do slider
/// @fn		static void TaskSliderScan(SLIDER_MODE_t eMode)
/// @Param	None    @brief  None
/// @return None    @brief  None
// *****************************************************************************
static void TaskSliderScan(SLIDER_MODE_t eMode)
{
    static uint16_t u16TimeBase = 0;
    uint16_t u16TimeNow = ReadSysTime();
    
    if (u16TimeNow != u16TimeBase)
    {
        u16TimeBase = u16TimeNow;
        
        if (xDeviceParams.u8DeviceOutputState)
        {     
            LED_SLIDER_0_ON;
            SliderRead(eMode, &xDeviceParams.u8DimLevel);
        }
        else
        {
            if (eMinDimAjustState == DIM_LIMITS_ADJUST_NONE)
            {
                SLIDER_OFF;
            }
            else 
            {
                SliderRead(eMode, &xDeviceParams.u8DimLevel);
            }
        }
    }
}

// *****************************************************************************
/// @brief	Tarefa que controla os leds da chave tactil
/// @fn		static void TaskSwitchLeds(void)
/// @Param	None    @brief None
/// @return None    @brief None
// *****************************************************************************
static void TaskSwitchLeds(void)
{
    if (xDeviceParams.u8DeviceOutputState)
    {
        BUTTON_LEDS_ON;
    }
    else
    {
        if (xDeviceParams.u8NightModeStatus)
        {
            BUTTON_LEDS_IN_NIGHT_MODE;
        }
        else
        {
            BUTTON_LEDS_OFF;
        }
    }
}

// *****************************************************************************
/// @brief	Tarefa de processamento da comunicação serial
/// @fn	    void TaskComm(void)
/// @Param	void   @brief void
/// @return void   @brief void
// *****************************************************************************
void TaskComm(void)
{
    UartProtocolResponseProcess(&xDeviceParams);   
}

// *****************************************************************************
/// @brief	Tarefa que trata o pressionamento da chave tactil
/// @fn		void SwitchScan(void)
/// @Param	None    @brief None
/// @return None    @brief None
// *****************************************************************************
void TaskSwitchScan(void)
{
    static bit bSwitchPressed = 0;
    static uint8_t u8PressTimesCount = 0;
    static uint16_t u16SwitchHoldedTimer = 0;
    static uint16_t u16FastPressTimer = 0;
        
    if (BUTTON_STATUS) // botão solto
    {
        if (bSwitchPressed == 1)
        {
            bSwitchPressed = 0;
            
            switch (eMinDimAjustState)
            {                
                case DIM_LIMITS_ADJUST_NONE:
                {
                    break;
                }
                case DIM_LIMITS_ADJUST_ADD:
                {
                    eSliderMode = SLIDER_MODE_CONFIG_MIN_DIM_LIMIT;    
                    eMinDimAjustState++;
                    return;
                }
                case DIM_LIMITS_ADJUST_MIN:
                {
                    SLIDER_OFF;
                    eSliderMode = SLIDER_MODE_CONFIG_MAX_DIM_LIMIT;
                    eMinDimAjustState++;
                    return;
                }
                case DIM_LIMITS_ADJUST_MAX:
                {
                    SLIDER_OFF;
                    eSliderMode = SLIDER_MODE_NORMAL;
                    eMinDimAjustState = DIM_LIMITS_ADJUST_NONE;
                    eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
                    return;
                }             
            }
                
            u8PressTimesCount++;
            xDeviceParams.u8DeviceOutputState = xDeviceParams.u8DeviceOutputState == OUTPUT_OFF ? OUTPUT_ON : OUTPUT_OFF;
            u16SwitchHoldedTimer = ReadSysTime();
        }
    }
    else // botão pressionado
    {
        bSwitchPressed = 1;
        u16FastPressTimer = ReadSysTime();
        
        if (TimeDiff(u16SwitchHoldedTimer) > SWITCH_HOLD_TIMEOUT) // trata funções de pressionamentos rápidos + retenção
        {
            switch (u8PressTimesCount)
            {
                case SWITCH_PRESS_TIMES_TO_ZIGBEE_LEAVE:
                {
                    xDeviceParams.u8FunctionFlags |= (1 << NETWORK_LEAVE_FLAG);
                    u16ButtomPwmDuty = GetDutyCycle(PWM21);  // GetDutyCycle é uma função personalizada implementada no driver do pwm
                    eConfigFeedbackState = CONFIG_FEEDBACK_ZIGBEE_LEAVE;
                    break;
                }
                case SWITCH_PRESS_TIMES_TO_SET_PHASE_CUT_MODE:
                {
                    xDeviceParams.u8PhaseCutMode = xDeviceParams.u8PhaseCutMode == PHASE_CUT_MODE_LEADING ? PHASE_CUT_MODE_TRAILING : PHASE_CUT_MODE_LEADING;
                    SLIDER_OFF;
                    eConfigFeedbackState = CONFIG_FEEDBACK_PHASE_CUT_MODE;
                    break;
                }
                case SWITCH_PRESS_TIMES_TO_SET_DIM_LIMITS:
                {
                    eMinDimAjustState = DIM_LIMITS_ADJUST_ADD;
                    SLIDER_OFF;
                    eConfigFeedbackState = CONFIG_FEEDBACK_DIM_LIMITS;
                    break;
                }
                case SWITCH_PRESS_TIMES_TO_FACTORY_RESET:
                {
                    SLIDER_OFF;
                    xDeviceParams.u8FunctionFlags |= (1 << FACTORY_RESET_FLAG);
                    eConfigFeedbackState = CONFIG_FEEDBACK_FACTORY_RESET;
                    break;
                }
            }
            
            u8PressTimesCount = 0;
        }
    }
    
    if (TimeDiff(u16FastPressTimer) > SWITCH_RELEASED_TIMOUT) // trata funções de pressionamentos rápidos
    {
        switch (u8PressTimesCount)
        {
            case SWITCH_PRESS_TIMES_TO_SET_NIGHT_MODE:
            {
                xDeviceParams.u8NightModeStatus = xDeviceParams.u8NightModeStatus == NIGHT_MODE_DISABLE ? NIGHT_MODE_ENABLE : NIGHT_MODE_DISABLE;
                eConfigFeedbackState = CONFIG_FEEDBACK_NIGHT_MODE;
                break;
            }
            case SWITCH_PRESS_TIMES_TO_SET_LOCK_MODE:
            {
                xDeviceParams.u8LockModeStatus = xDeviceParams.u8LockModeStatus == LOCK_MODE_DISABLE ? LOCK_MODE_ENABLE : LOCK_MODE_DISABLE;
                SLIDER_OFF;
                eConfigFeedbackState = CONFIG_FEEDBACK_LOCK_MODE;
                break;
            }
        }
        
        u8PressTimesCount = 0;        
    }
}

// *****************************************************************************
/// @brief	Tarefa que trata as opções de configuração do dispositivo
/// @fn		void TaskConfigHandle(void)
/// @Param	None    @brief  None
/// @return None    @brief  None
// *****************************************************************************
void TaskConfigHandle(void)
{
    static uint8_t u8BlinkTimes = 0;
    static uint8_t u8Count = 0;
    static uint16_t u16Timer = 0;
    static bit bSliderDirection = 0;
    
    switch (eConfigFeedbackState)
    {
        case CONFIG_FEEDBACK_IDLE:
        {
            TaskSliderScan(SLIDER_MODE_NORMAL);
            TaskSwitchLeds();
            break;
        }
        case CONFIG_FEEDBACK_PHASE_CUT_MODE:
        {            
            u8BlinkTimes = PHASE_CUT_CHANGE_BLINK_TIMES;
            
            if (TimeDiff(u16Timer) > DEFAULT_LED_BLINK_DELAY)
            {
                u16Timer = ReadSysTime();
                
                if (xDeviceParams.u8PhaseCutMode == PHASE_CUT_MODE_LEADING)
                {
                    SLIDER_TOGGLE_FOR_LEADING_MODE;
                }
                else
                {
                    SLIDER_TOGGLE_FOR_TRAILING_MODE;
                }
                
                if (++u8Count == u8BlinkTimes)
                {                
                    u8Count = 0;
                    u8BlinkTimes = 0;
                    eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
                }
            }
            break;
        }
        case CONFIG_FEEDBACK_DIM_LIMITS:
        {  
            if (TimeDiff(u16Timer) > DEFAULT_LED_BLINK_DELAY)
            {  
                u16Timer = ReadSysTime();     
                
                switch (eMinDimAjustState)
                {
                    case DIM_LIMITS_ADJUST_NONE:
                    {
                        break;
                    }
                    case DIM_LIMITS_ADJUST_ADD:
                    case DIM_LIMITS_ADJUST_MIN:
                    {
                        eSliderMode = SLIDER_MODE_CONFIG_MIN_DIM_LIMIT;
                        LED_SLIDER_1_TOGGLE;
                        break;
                    }
                    case DIM_LIMITS_ADJUST_MAX:
                    {
                        eSliderMode = SLIDER_MODE_CONFIG_MAX_DIM_LIMIT;
                        LED_SLIDER_10_TOGGLE;                        
                        break;
                    }
                }                
            }
            TaskSliderScan(eSliderMode);
            break;
        }
        case CONFIG_FEEDBACK_LOCK_MODE:
        {          
            if (xDeviceParams.u8LockModeStatus == LOCK_MODE_DISABLE)
            {
                u8BlinkTimes = LOCK_MODE_DISABLE_BLINK_TIMES;
            }
            else
            {
                u8BlinkTimes = LOCK_MODE_ENABLE_BLINK_TIMES;
            }
            
            if (TimeDiff(u16Timer) > DEFAULT_LED_BLINK_DELAY)
            {
                u16Timer = ReadSysTime();
                SLIDER_TOGGLE;
                
                if (++u8Count == u8BlinkTimes)
                {                
                    u8Count = 0;
                    u8BlinkTimes = 0;
                    eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
                }
            }
            break;
        }
        case CONFIG_FEEDBACK_NIGHT_MODE:
        {            
            eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
            break;
        }
        case CONFIG_FEEDBACK_FACTORY_RESET:
        {
            u8BlinkTimes = FACTORY_RESET_BLINK_TIMES;
            
            if (TimeDiff(u16Timer) > DEFAULT_LED_BLINK_DELAY)
            {
                u16Timer = ReadSysTime();
                SLIDER_TOGGLE;
                
                if (++u8Count == u8BlinkTimes)
                {                
                    u8Count = 0;
                    u8BlinkTimes = 0;
                    eConfigFeedbackState = CONFIG_FEEDBACK_IDLE;
                }
            }
            break;
        }
        case CONFIG_FEEDBACK_ZIGBEE_LEAVE:
        {
            if (++u8Count == 3) // atraso para o incremento do pwm
            {                
                u8Count = 0;
                
                if (bSliderDirection)
                {
                    if (u16ButtomPwmDuty < BUTTON_LEDS_MAX_COUNT_VALUE)
                    {
                       u16ButtomPwmDuty++;
                    }
                    else
                    {
                        bSliderDirection = 0;
                    }                
                }
                else
                {
                    if (u16ButtomPwmDuty > 0)
                    {
                       u16ButtomPwmDuty--;
                    }
                    else
                    {
                        bSliderDirection = 1;
                    }  
                }       
                
                BUTTON_LEDS_SET_COUNTER(u16ButtomPwmDuty);
            }
            break;
        }
    }
}

// *****************************************************************************
/// @brief	Função que executa sinalização de network leave recebida do mestre
/// @fn		void TaskConfigHandle(void)
/// @Param	None    @brief  None
/// @return None    @brief  None
// *****************************************************************************
void ExecuteNetworkLeave(void)
{
    eConfigFeedbackState = CONFIG_FEEDBACK_ZIGBEE_LEAVE;
}

// *****************************************************************************
/// @brief	Função que executa sinalização de factory reset recebida do mestre
/// @fn		void TaskConfigHandle(void)
/// @Param	None    @brief  None
/// @return None    @brief  None
// *****************************************************************************
void ExecuteFactoryReset(void)
{
    eConfigFeedbackState = CONFIG_FEEDBACK_FACTORY_RESET;    
}
