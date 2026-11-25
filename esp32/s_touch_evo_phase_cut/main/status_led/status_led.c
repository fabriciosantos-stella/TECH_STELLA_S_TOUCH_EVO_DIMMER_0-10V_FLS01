/*
 *  status_led.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "status_led.h"
#include "hardware_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Private define ----------------------------------------------------------------------------------------
#define STATUS_LED_TASK_STACK_SIZE              2048
#define STATUS_LED_TASK_PRIORITY                2

#define STATUS_LED_NORMAL_INTERVAL_MS           500
#define STATUS_LED_ERROR_INTERVAL_MS            100

#define STATUS_LED_TEMP_ON_TIME_MS              100
#define STATUS_LED_TEMP_OFF_TIME_MS             100
#define STATUS_LED_TEMP_PAUSE_TIME_MS           1000

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------

// Private variables -------------------------------------------------------------------------------------
static STATUS_LED_MODE_t eStatusLedMode = STATUS_LED_MODE_NORMAL;
static portMUX_TYPE StatusLedMux = portMUX_INITIALIZER_UNLOCKED;

// Private function prototypes ---------------------------------------------------------------------------

// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Tarefa que controla o led de status
* @fn       static void StatusLedTask(void *pvParameters)   
* @param    *pvParameters   @brief ponteiro com os parametros da tarefa
* @return   void            @brief void
*********************************************************************************************************/
static void StatusLedTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    STATUS_LED_MODE_t eCurrentMode;
    uint8_t u8LedState = 0;
    uint8_t u8TemperaturePhase = 0;
    uint32_t u32IntervalMs;
    (void)pvParameters;

    while (1)
    {
        portENTER_CRITICAL(&StatusLedMux);
        eCurrentMode = eStatusLedMode;
        portEXIT_CRITICAL(&StatusLedMux);

        switch (eCurrentMode)
        {
            case STATUS_LED_MODE_NORMAL:
            {
                u8TemperaturePhase = 0;
                u8LedState ^= 1;
                u32IntervalMs = STATUS_LED_NORMAL_INTERVAL_MS;
                break;
            }
            case STATUS_LED_SLAVE_COMM_ERROR:
            {
                u8TemperaturePhase = 0;
                u8LedState ^= 1;
                u32IntervalMs = STATUS_LED_ERROR_INTERVAL_MS;
                break;
            }
            case STATUS_LED_TEMPERATURE_ERROR:
            {
                switch (u8TemperaturePhase)
                {
                    case 0:
                    {
                        u8LedState = 1;
                        u32IntervalMs = STATUS_LED_TEMP_ON_TIME_MS;
                        break;
                    }
                    case 1:
                    {
                        u8LedState = 0;
                        u32IntervalMs = STATUS_LED_TEMP_OFF_TIME_MS;
                        break;
                    }
                    case 2:
                    {
                        u8LedState = 1;
                        u32IntervalMs = STATUS_LED_TEMP_ON_TIME_MS;
                        break;
                    }
                    default:
                    {
                        u8LedState = 0;
                        u32IntervalMs = STATUS_LED_TEMP_PAUSE_TIME_MS;
                        break;
                    }
                }

                u8TemperaturePhase = (u8TemperaturePhase + 1) % 4;
                break;
            }
            default:
            {
                u8LedState = 0;
                u32IntervalMs = STATUS_LED_NORMAL_INTERVAL_MS;
                break;
            }
        }

        gpio_set_level(STATUS_LED_GPIO, u8LedState ? STATUS_LED_ACTIVE_LEVEL : STATUS_LED_INACTIVE_LEVEL);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(u32IntervalMs));
    }
}

/*********************************************************************************************************
* @brief    Função que inicializa e configura o pino do led de status
* @fn       esp_err_t StatusLedInit(void)
* @param    void         @brief void
* @return   esp_err_t    @brief retorna o status de erro ou sucesso
*********************************************************************************************************/
esp_err_t StatusLedInit(void)
{
    gpio_config_t xGpioConfig =
    {
        .pin_bit_mask = (1ULL << STATUS_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t xError = gpio_config(&xGpioConfig);

    if (xError != ESP_OK)
    {
        return xError;
    }

    gpio_set_level(STATUS_LED_GPIO, STATUS_LED_INACTIVE_LEVEL);
    eStatusLedMode = STATUS_LED_MODE_NORMAL;
    BaseType_t xResult = xTaskCreate(StatusLedTask, "StatusLedTask", STATUS_LED_TASK_STACK_SIZE, NULL, STATUS_LED_TASK_PRIORITY, NULL);

    if (xResult != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Configura o modo de piscadas do led de status
* @fn       void StatusLedSetMode(STATUS_LED_MODE_t eMode)
* @param    eMode       @brief modo do LED
* @return   void        @brief void
*********************************************************************************************************/
void StatusLedSetMode(STATUS_LED_MODE_t eMode)
{
    if ((eMode < STATUS_LED_MODE_NORMAL) || (eMode > STATUS_LED_TEMPERATURE_ERROR))
    {
        return;
    }

    portENTER_CRITICAL(&StatusLedMux);
    eStatusLedMode = eMode;
    portEXIT_CRITICAL(&StatusLedMux);
}
