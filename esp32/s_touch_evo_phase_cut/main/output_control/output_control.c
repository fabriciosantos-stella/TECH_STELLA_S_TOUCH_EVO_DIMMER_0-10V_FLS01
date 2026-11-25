/*
 *  output_control.c
 *
 *  Created on: Nov 10, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "output_control.h"
#include "hardware_config.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Private define ----------------------------------------------------------------------------------------
#define DIMMER_MAX_DUTY  ((1UL << DIMMER_0_10V_PWM_RESOLUTION_BITS) - 1)
#define DIMMER_MIN_DUTY  ((uint32_t)(0.12f * DIMMER_MAX_DUTY))  // ~1.2 V offset no nivel 1
#define AUX_PIN          GPIO_NUM_7
#define DIMMERMODE1_10         GPIO_NUM_14

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static const char *TAG = "OUTPUT_CTRL";
static uint8_t s_u8AuxState = 0;
static uint8_t s_u8DimmerModeState = 0;
static uint8_t s_u8CurrentPercent = 0;

// Private function prototypes ---------------------------------------------------------------------------
// Private functions -------------------------------------------------------------------------------------


/*********************************************************************************************************
* @brief    Inicializa o controle de saída (dimmer 0-10V e pino auxiliar)
* @fn       esp_err_t OutputControlInit(void)
* @param    void        @brief void
* @return   esp_err_t   @brief ESP_OK em caso de sucesso
*********************************************************************************************************/
esp_err_t OutputControlInit(void)
{
    // Configuração do PWM para dimmer 0-10V
    ledc_timer_config_t xTimerConfig =
    {
        .speed_mode       = DIMMER_0_10V_PWM_MODE,
        .duty_resolution  = DIMMER_0_10V_PWM_RESOLUTION_BITS,
        .timer_num        = DIMMER_0_10V_PWM_TIMER,
        .freq_hz          = DIMMER_0_10V_PWM_FREQUENCY_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    esp_err_t xError = ledc_timer_config(&xTimerConfig);
    if (xError != ESP_OK) return xError;

    ledc_channel_config_t xChannelConfig =
    {
        .gpio_num       = DIMMER_0_10V_GPIO,
        .speed_mode     = DIMMER_0_10V_PWM_MODE,
        .channel        = DIMMER_0_10V_PWM_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = DIMMER_0_10V_PWM_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };

    xError = ledc_channel_config(&xChannelConfig);
    if (xError != ESP_OK) return xError;

    // Configuração do pino auxiliar (GPIO 7)
    gpio_reset_pin(AUX_PIN);
    gpio_set_direction(AUX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(AUX_PIN, 0);
    s_u8AuxState = 0;


    gpio_reset_pin(DIMMERMODE1_10);
    gpio_set_direction(DIMMERMODE1_10, GPIO_MODE_OUTPUT);
    gpio_set_level(DIMMERMODE1_10, 0);
    s_u8DimmerModeState = 0;

    ESP_LOGI(TAG, "Pino14 %d initialized", DIMMERMODE1_10);

    ESP_LOGI(TAG, "Aux pin %d initialized", AUX_PIN);

    
    return ESP_OK;
}


/*********************************************************************************************************
* @brief    Define o nível do dimmer 0-10V (0-11, compatibilidade)
* @fn       esp_err_t OutputControlSetLevel(uint8_t u8Level)
* @param    u8Level     @brief Nível de 0 a 11
* @return   esp_err_t   @brief ESP_OK em caso de sucesso
*********************************************************************************************************/
esp_err_t OutputControlSetLevel(uint8_t u8Level)
{
    if (u8Level > 11)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t u8Percent = u8Level * 10;
    if (u8Percent > 100)
    {
        u8Percent = 100;
    }

    OutputControlSetPercent(u8Percent);
    ESP_LOGI(TAG, "Dimmer percent: %u%%", u8Percent);

    return ESP_OK;
}


/*********************************************************************************************************
* @brief    Define o percentual do dimmer e aplica imediatamente (0-100%)
* @fn       void OutputControlSetPercent(uint8_t u8Percent)
* @param    u8Percent   @brief Percentual desejado de 0 a 100
* @return   void        @brief void
*********************************************************************************************************/
void OutputControlSetPercent(uint8_t u8Percent)
{
    if (u8Percent > 100)
    {
        u8Percent = 100;
    }

    s_u8CurrentPercent = u8Percent;

    uint32_t u32Duty = 0;

    if (u8Percent > 0)
    {
        u32Duty = DIMMER_MIN_DUTY + (((u8Percent - 1) * (DIMMER_MAX_DUTY - DIMMER_MIN_DUTY)) / 99);
    }

    ledc_set_duty(DIMMER_0_10V_PWM_MODE, DIMMER_0_10V_PWM_CHANNEL, u32Duty);
    ledc_update_duty(DIMMER_0_10V_PWM_MODE, DIMMER_0_10V_PWM_CHANNEL);

    //ESP_LOGI(TAG, "Dimmer percent: %u%%", u8Percent);

}


/*********************************************************************************************************
* @brief    Retorna o percentual atual do dimmer em 0-100%
* @fn       uint8_t OutputControlGetCurrentPercent(void)
* @param    void        @brief void
* @return   uint8_t     @brief Percentual atual de 0 a 100
*********************************************************************************************************/
uint8_t OutputControlGetCurrentPercent(void)
{
    return s_u8CurrentPercent;
}


/*********************************************************************************************************
* @brief    Define o estado do pino SK1 (GPIO 7)
* @fn       void OutputControlSetSK1(uint8_t u8State)
* @param    u8State     @brief 0 = desligado, diferente de 0 = ligado
* @return   void        @brief void
*********************************************************************************************************/
void OutputControlSetSK1(uint8_t u8State)
{
    s_u8AuxState = (u8State != 0) ? 1 : 0;
    gpio_set_level(AUX_PIN, s_u8AuxState);

    ESP_LOGI(TAG, "SK1 pin %d = %d", AUX_PIN, s_u8AuxState);
}


/*********************************************************************************************************
* @brief    Inverte o estado do pino SK1 (GPIO 7)
* @fn       void OutputControlToggleSK1(void)
* @param    void        @brief void
* @return   void        @brief void
*********************************************************************************************************/
void OutputControlToggleSK1(void)
{
    OutputControlSetSK1(!s_u8AuxState);
}


/*********************************************************************************************************
* @brief    Retorna o estado atual do pino SK1 (GPIO 7)
* @fn       uint8_t OutputControlGetSK1State(void)
* @param    void        @brief void
* @return   uint8_t     @brief 0 = desligado, 1 = ligado
*********************************************************************************************************/
uint8_t OutputControlGetSK1State(void)
{
    return s_u8AuxState;
}


void OutputControlSetDimmerMode(uint8_t u8State)
{
s_u8DimmerModeState = (u8State != 0) ? 1 : 0;
gpio_set_level(DIMMERMODE1_10, s_u8DimmerModeState);

ESP_LOGI(TAG, "Pino14 %d = %d", DIMMERMODE1_10, s_u8DimmerModeState);
}