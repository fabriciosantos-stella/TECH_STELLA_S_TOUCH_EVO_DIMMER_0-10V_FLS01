/*
 *  temperature_manager.c
 *
 *  Created on: Nov 12, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "temperature_manager.h"
#include "hardware_config.h"
#include "telemetry_manager.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/temperature_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include <math.h>

// Private define ----------------------------------------------------------------------------------------
#define TEMPERATURE_MANAGER_TASK_STACK_SIZE       3072
#define TEMPERATURE_MANAGER_TASK_PRIORITY         4
#define TEMPERATURE_MANAGER_PERIOD_MS             500
#define TEMPERATURE_MANAGER_NTC_SAMPLES           8

#define NTC_FIXED_RESISTOR_RESISTANCE             10000.0f
#define NTC_NOMINAL_RESISTANCE                    10000.0f
#define KELVIN_TEMPERATURE                        273.15f
#define NTC_NOMINAL_TEMPERATURE                   (KELVIN_TEMPERATURE + 25.0f) // Temperatura Kelvin + temperatua ambiente
#define NTC_BETA_VALUE                            3380.0f
#define NTC_SUPPLY_VOLTAGE                        3300.0f

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static adc_oneshot_unit_handle_t xAdcHandle = NULL;
static temperature_sensor_handle_t xTemperatureSensorHandle = NULL;
static SemaphoreHandle_t xTemperatureMutex = NULL;
static adc_cali_handle_t xAdcCalibrationHandle = NULL;
static TEMPERATURE_DATA_t xTemperatureData =
{
    .iNtcRawValue               = 0,
    .fInternalTemperature      = 0.0f,
    .bNtcValid                  = false,
    .bInternalTemperatureValid  = false
};

// Private function prototypes ---------------------------------------------------------------------------
static void TemperatureManagerTask(void *pvParameters);
static esp_err_t TemperatureManagerReadNtc(int *piRawValue);
static esp_err_t TemperatureManagerConvertNtcToCelsius(int iRawValue, float *pfTemperature);

// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Realiza a leitura e média das amostras do ADC
* @fn       static esp_err_t TemperatureManagerReadNtc(int *piRawValue)
* @param    piRawValue    @brief Ponteiro para armazenar o resultado
* @return   esp_err_t     @brief status da resposta, sucesso ou falha
*********************************************************************************************************/
static esp_err_t TemperatureManagerReadNtc(int *piRawValue)
{
    int iSample;
    int32_t i32Accumulator = 0;

    if (piRawValue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    for (uint8_t u8Index = 0; u8Index < TEMPERATURE_MANAGER_NTC_SAMPLES; u8Index++)
    {
        esp_err_t xError = adc_oneshot_read(xAdcHandle, NTC_ADC_CHANNEL, &iSample);

        if (xError != ESP_OK)
        {
            return xError;
        }

        i32Accumulator += iSample;
    }

    *piRawValue = (int)(i32Accumulator / TEMPERATURE_MANAGER_NTC_SAMPLES);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Tarefa de monitoramento periódico das temperaturas
* @fn       static void TemperatureManagerTask(void *pvParameters)
* @param    pvParameters   @brief Parâmetros da tarefa
* @return   void    @brief void
*********************************************************************************************************/
static void TemperatureManagerTask(void *pvParameters)
{
    (void)pvParameters;
    int iNtcRawValue;
    float fNtcTemperature;
    float fInternalTemperature;
    esp_err_t xNtcError;
    esp_err_t xInternalError;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        xNtcError = TemperatureManagerReadNtc(&iNtcRawValue);

        if (xNtcError == ESP_OK)
        {
            xNtcError = TemperatureManagerConvertNtcToCelsius(iNtcRawValue, &fNtcTemperature);
        }

        xInternalError = temperature_sensor_get_celsius(xTemperatureSensorHandle, &fInternalTemperature);

        if (xSemaphoreTake(xTemperatureMutex, portMAX_DELAY) == pdTRUE)
        {
            xTemperatureData.bNtcValid = (xNtcError == ESP_OK);

            if (xNtcError == ESP_OK)
            {
                xTemperatureData.iNtcRawValue = iNtcRawValue;
                xTemperatureData.fNtcTemperature = fNtcTemperature;
            }

            xTemperatureData.bInternalTemperatureValid = (xInternalError == ESP_OK);

            if (xInternalError == ESP_OK)
            {
                xTemperatureData.fInternalTemperature = fInternalTemperature;
            }

            xSemaphoreGive(xTemperatureMutex);
        }

        if ((xNtcError == ESP_OK) && (xInternalError == ESP_OK))
        {
            TelemetryManagerUpdateTemperatures(xTemperatureData.fNtcTemperature, xTemperatureData.fInternalTemperature); // atualiza o módulo de telemetria
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMPERATURE_MANAGER_PERIOD_MS));
    }
}

/*********************************************************************************************************
* @brief    Inicializa os sensores e cria a tarefa de monitoramento
* @fn       esp_err_t TemperatureManagerInit(void) 
* @param    void    @brief void
* @return   esp_err_t     @brief status da resposta, sucesso ou falha
*********************************************************************************************************/
esp_err_t TemperatureManagerInit(void)
{
    esp_err_t xError = ESP_OK;

    if ((xAdcHandle != NULL) || (xTemperatureSensorHandle != NULL) || (xTemperatureMutex != NULL))
    {
        return ESP_ERR_INVALID_STATE;
    }

    xTemperatureMutex = xSemaphoreCreateMutex();    // Cria mutex

    if (xTemperatureMutex == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    adc_oneshot_unit_init_cfg_t xAdcUnitConfig =
    {
        .unit_id = NTC_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    xError = adc_oneshot_new_unit(&xAdcUnitConfig, &xAdcHandle);

    if (xError == ESP_OK)
    {
        adc_oneshot_chan_cfg_t xAdcChannelConfig =
        {
            .atten = NTC_ADC_ATTENUATION,
            .bitwidth = NTC_ADC_BITWIDTH
        };

        xError = adc_oneshot_config_channel(xAdcHandle, NTC_ADC_CHANNEL, &xAdcChannelConfig);
    }

    if (xError == ESP_OK)
    {
        adc_cali_curve_fitting_config_t xAdcCalibrationConfig =
        {
            .unit_id = NTC_ADC_UNIT,
            .chan = NTC_ADC_CHANNEL,
            .atten = NTC_ADC_ATTENUATION,
            .bitwidth = NTC_ADC_BITWIDTH
        };

        xError = adc_cali_create_scheme_curve_fitting(&xAdcCalibrationConfig, &xAdcCalibrationHandle);
    }

    if (xError == ESP_OK)
    {
        temperature_sensor_config_t xTemperatureConfig =
        {
            .range_min = INTERNAL_TEMP_RANGE_MIN_C,
            .range_max = INTERNAL_TEMP_RANGE_MAX_C
        };

        xError = temperature_sensor_install(&xTemperatureConfig, &xTemperatureSensorHandle);
    }

    if (xError == ESP_OK)
    {
        xError = temperature_sensor_enable(xTemperatureSensorHandle);
    }

    if (xError == ESP_OK)
    {
        BaseType_t xResult = xTaskCreate(TemperatureManagerTask, "TemperatureManagerTask", TEMPERATURE_MANAGER_TASK_STACK_SIZE, NULL, TEMPERATURE_MANAGER_TASK_PRIORITY, NULL);

        if (xResult != pdPASS)
        {
            xError = ESP_ERR_NO_MEM;
        }
    }

    if (xError != ESP_OK)
    {
        if (xTemperatureSensorHandle != NULL)
        {
            temperature_sensor_uninstall(xTemperatureSensorHandle);
            xTemperatureSensorHandle = NULL;
        }

        if (xAdcHandle != NULL)
        {
            adc_oneshot_del_unit(xAdcHandle);
            xAdcHandle = NULL;
        }

        if (xTemperatureMutex != NULL)
        {
            vSemaphoreDelete(xTemperatureMutex);
            xTemperatureMutex = NULL;
        }

        if (xAdcCalibrationHandle != NULL)
        {
            adc_cali_delete_scheme_curve_fitting(xAdcCalibrationHandle);
            xAdcCalibrationHandle = NULL;
        }
    }

    return xError;
}

/*********************************************************************************************************
* @brief    Retorna os últimos dados adquiridos pelos sensores
* @fn       esp_err_t TemperatureManagerGetData(TEMPERATURE_DATA_t *pxData)
* @param    pxData    @brief Ponteiro para a estrutura de dados
* @return   esp_err_t   @brief status da resposta, sucesso ou falha
*********************************************************************************************************/
esp_err_t TemperatureManagerGetData(TEMPERATURE_DATA_t *pxData)
{
    if (pxData == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xTemperatureMutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(xTemperatureMutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    *pxData = xTemperatureData;

    xSemaphoreGive(xTemperatureMutex);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Converte a leitura ADC do NTC para graus Celsius
* @param    iRawValue       @brief Valor bruto do ADC
* @param    pfTemperature  @brief Ponteiro para armazenar a temperatura
* @return   esp_err_t   @brief status da resposta, sucesso ou falha
*********************************************************************************************************/
static esp_err_t TemperatureManagerConvertNtcToCelsius(int iRawValue, float *pfTemperature)
{
    int iVoltageMv;
    float fNtcResistance;
    float fTemperatureK;

    if (pfTemperature == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xAdcCalibrationHandle == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t xError = adc_cali_raw_to_voltage(xAdcCalibrationHandle, iRawValue, &iVoltageMv); // converte valor bruto do ADC para tensão calibrada

    if (xError != ESP_OK)
    {
        return xError;
    }
    
    if ((iVoltageMv <= 0) || (iVoltageMv >= NTC_SUPPLY_VOLTAGE)) // verifica se a tensão está dentro da faixa válida do divisor
    {
        return ESP_ERR_INVALID_RESPONSE;
    }
    
    fNtcResistance = NTC_FIXED_RESISTOR_RESISTANCE * ((float)iVoltageMv / (NTC_SUPPLY_VOLTAGE - (float)iVoltageMv)); // calcula a resistência do NTC  
    fTemperatureK = 1.0f / ((1.0f / NTC_NOMINAL_TEMPERATURE) + (logf(fNtcResistance / NTC_NOMINAL_RESISTANCE) / NTC_BETA_VALUE)); // aplica a equação Beta    
    *pfTemperature = fTemperatureK - KELVIN_TEMPERATURE; // converte Kelvin para Celsius

    return ESP_OK;
}
