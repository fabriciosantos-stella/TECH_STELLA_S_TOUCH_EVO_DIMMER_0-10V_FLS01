/*
 *  phase_control.c
 *
 *  Created on: Nov 10, 2026
 *      Author: rodrigo.arnhold
 *
 

// Includes ----------------------------------------------------------------------------------------------
#include "phase_control.h"
#include "hardware_config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "telemetry_manager.h"

// Private define ----------------------------------------------------------------------------------------
#define PHASE_CONTROL_DISCARD_SAMPLES       10
#define PHASE_CONTROL_CALIBRATION_SAMPLES   10

#define PHASE_CONTROL_MIN_HALF_CYCLE_US     7000
#define PHASE_CONTROL_MAX_HALF_CYCLE_US     11000

#define PHASE_CONTROL_FILTER_SHIFT          6

#define PHASE_CONTROL_US_PER_SECOND         1000000.0f
#define PHASE_CONTROL_HALF_CYCLES           2.0f

#define PHASE_CONTROL_TASK_STACK_SIZE       2048
#define PHASE_CONTROL_TASK_PRIORITY         5

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static const char *TAG = "PHASE_CONTROL";
static portMUX_TYPE s_phaseMux = portMUX_INITIALIZER_UNLOCKED;
static int64_t s64LastZeroCrossingUs = 0;
static uint32_t u32DiscardSamples = 0;
static uint32_t u32CalibrationSamples = 0;
static uint64_t u64CalibrationAccumulator = 0;
static uint32_t u32FilteredHalfCycleUs = 0;
static bool bFrequencyValid = false;
static TaskHandle_t xPhaseControlTaskHandle = NULL;

// Private function prototypes ---------------------------------------------------------------------------
static void PhaseControlTask(void *pvParameters);

// Private functions -------------------------------------------------------------------------------------
*********************************************************************************************************
* @brief    Função que trata a interrução de zero crossing e mede o periodo do semiciclo
* @fn       static void PhaseControlZeroCrossingISR(void *pvArgument)
* @param    *pvArgument      @brief ponteiro para o argumento da interrupção
* @return   void            @brief void
********************************************************************************************************
static void PhaseControlZeroCrossingISR(void *pvArgument)
{
    int64_t s64NowUs = esp_timer_get_time();

    if (s64LastZeroCrossingUs == 0)
    {
        s64LastZeroCrossingUs = s64NowUs;
        return;
    }

    uint32_t u32MeasuredHalfCycleUs = (uint32_t)(s64NowUs - s64LastZeroCrossingUs);
    
    if (u32MeasuredHalfCycleUs < PHASE_CONTROL_MIN_HALF_CYCLE_US)
    {
        return;
    }
    
    if (u32MeasuredHalfCycleUs > PHASE_CONTROL_MAX_HALF_CYCLE_US)
    {
        s64LastZeroCrossingUs = s64NowUs;
        return;
    }

    s64LastZeroCrossingUs = s64NowUs;
    
    if (u32DiscardSamples < PHASE_CONTROL_DISCARD_SAMPLES)
    {
        u32DiscardSamples++;
        return;
    }
   
    if (!bFrequencyValid) 
    {
        u64CalibrationAccumulator += u32MeasuredHalfCycleUs;
        u32CalibrationSamples++;

        if (u32CalibrationSamples >= PHASE_CONTROL_CALIBRATION_SAMPLES)
        {
            portENTER_CRITICAL_ISR(&s_phaseMux);

            u32FilteredHalfCycleUs = (uint32_t)(u64CalibrationAccumulator / PHASE_CONTROL_CALIBRATION_SAMPLES);
            bFrequencyValid = true;

            portEXIT_CRITICAL_ISR(&s_phaseMux);

            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            vTaskNotifyGiveFromISR(xPhaseControlTaskHandle, &xHigherPriorityTaskWoken);

            if (xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }
        }

        return;
    }
    
    // aplica filtro IIR para estabilizar a saída do dimmer e evitar oscilações na dimerização
    portENTER_CRITICAL_ISR(&s_phaseMux);// IIR filter: alpha = 1/64.

    int32_t s32Error = (int32_t)u32MeasuredHalfCycleUs - (int32_t)u32FilteredHalfCycleUs;

    u32FilteredHalfCycleUs += (int32_t)(s32Error / (1 << PHASE_CONTROL_FILTER_SHIFT));

    portEXIT_CRITICAL_ISR(&s_phaseMux);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(xPhaseControlTaskHandle, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

*********************************************************************************************************
* @brief    Função de inicialização e configuração do pino de interrupção do zero-crossing
* @fn       esp_err_t PhaseControlInit(void)
* @param    void      @brief void
* @return   esp_err_t @brief status da estrutura de erros do sistema
********************************************************************************************************
esp_err_t PhaseControlInit(void)
{
    esp_err_t eResult;
    BaseType_t xTaskResult;
    s64LastZeroCrossingUs = 0;
    u32DiscardSamples = 0;
    u32CalibrationSamples = 0;
    u64CalibrationAccumulator = 0;
    u32FilteredHalfCycleUs = 0;
    bFrequencyValid = false;

    gpio_config_t xGpioConfig =
    {
        .pin_bit_mask = (1ULL << ZERO_CROSSING_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    eResult = gpio_config(&xGpioConfig);

    if (eResult != ESP_OK)
    {
        ESP_LOGE(TAG, "GPIO configuration failed: %s", esp_err_to_name(eResult));
        return eResult;
    }

    // Instala o serviço de interrupções GPIO.
    eResult = gpio_install_isr_service(0);

    if ((eResult != ESP_OK) && (eResult != ESP_ERR_INVALID_STATE))
    {
        ESP_LOGE(TAG, "ISR service installation failed: %s", esp_err_to_name(eResult));
        return eResult;
    }

    // Cria a tarefa antes de habilitar a interrupção
    xTaskResult = xTaskCreate(PhaseControlTask, "PhaseControlTask", PHASE_CONTROL_TASK_STACK_SIZE, NULL, PHASE_CONTROL_TASK_PRIORITY, &xPhaseControlTaskHandle);

    if (xTaskResult != pdPASS)
    {
        ESP_LOGE(TAG, "Phase Control task creation failed");
        xPhaseControlTaskHandle = NULL;
        return ESP_FAIL;
    }

    // Registra a rotina de interrupção no GPIO18
    eResult = gpio_isr_handler_add(ZERO_CROSSING_GPIO, PhaseControlZeroCrossingISR, NULL);

    if (eResult != ESP_OK)
    {
        ESP_LOGE(TAG, "Zero crossing ISR registration failed: %s", esp_err_to_name(eResult));
        vTaskDelete(xPhaseControlTaskHandle);
        xPhaseControlTaskHandle = NULL;

        return eResult;
    }

    // Configura as duas bordas como fontes de interrupção
    eResult = gpio_set_intr_type(ZERO_CROSSING_GPIO, ZERO_CROSSING_INTR_TYPE);

    if (eResult != ESP_OK)
    {
        ESP_LOGE(TAG, "Zero crossing interrupt configuration failed: %s", esp_err_to_name(eResult));
        gpio_isr_handler_remove(ZERO_CROSSING_GPIO);
        vTaskDelete(xPhaseControlTaskHandle);
        xPhaseControlTaskHandle = NULL;

        return eResult;
    }

    // Habilita a interrupção somente após toda a configuração.
    eResult = gpio_intr_enable(ZERO_CROSSING_GPIO);

    if (eResult != ESP_OK)
    {
        ESP_LOGE(TAG, "Zero crossing interrupt enable failed: %s", esp_err_to_name(eResult));
        gpio_isr_handler_remove(ZERO_CROSSING_GPIO);
        vTaskDelete(xPhaseControlTaskHandle);
        xPhaseControlTaskHandle = NULL;

        return eResult;
    }

    ESP_LOGI(TAG, "Phase control initialized");

    return ESP_OK;
}

*********************************************************************************************************
* @brief    Função de inicialização e configuração do pino de interrupção do zero-crossing
* @fn       esp_err_t PhaseControlGetData(PHASE_CONTROL_DATA_t *pxData)
* @param    *pxData        @brief ponteiro para a estrutura de parametros do phase control
* @return   esp_err_t      @brief status da estrutura de erros do sistema
********************************************************************************************************
esp_err_t PhaseControlGetData(PHASE_CONTROL_DATA_t *pxData)
{
    if (pxData == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t u32HalfCycleUs;
    bool bValid;

    portENTER_CRITICAL(&s_phaseMux);

    u32HalfCycleUs = u32FilteredHalfCycleUs;
    bValid = bFrequencyValid;

    portEXIT_CRITICAL(&s_phaseMux);

    pxData->u32HalfCycleTimeUs = u32HalfCycleUs;
    pxData->bFrequencyValid = bValid;

    if (bValid && (u32HalfCycleUs > 0))
    {
        pxData->fLineFrequencyHz = PHASE_CONTROL_US_PER_SECOND / (PHASE_CONTROL_HALF_CYCLES * (float)u32HalfCycleUs);
    }
    else
    {
        pxData->fLineFrequencyHz = 0.0f;
    }

    return ESP_OK;
}

*********************************************************************************************************
* @brief    Tarefa responsável pelo processamento da frequência da rede e atualização da telemetria
* @fn       static void PhaseControlTask(void *pvParameters)
* @param    pvParameters    @brief Ponteiro para parâmetros da tarefa
* @return   void            @brief void
********************************************************************************************************
static void PhaseControlTask(void *pvParameters)
{
    (void)pvParameters;

    PHASE_CONTROL_DATA_t xPhaseData;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // aguarda uma notificação enviada pela ISR de zero crossing

        if (PhaseControlGetData(&xPhaseData) == ESP_OK)
        {
            if (xPhaseData.bFrequencyValid)
            {
                TelemetryManagerUpdateFrequency(xPhaseData.fLineFrequencyHz);
            }
        }
    }
}
*/