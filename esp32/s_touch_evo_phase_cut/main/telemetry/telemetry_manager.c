/*
 *  telemetry_manager.c
 *
 *  Created on: Nov 12, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "telemetry_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// Private define ----------------------------------------------------------------------------------------
// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static TELEMETRY_DATA_t xTelemetryData = {0};
static portMUX_TYPE s_telemetryMux = portMUX_INITIALIZER_UNLOCKED;

// Private function prototypes ---------------------------------------------------------------------------
// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Função de inicialização do Telemetry Manager
* @fn       esp_err_t TelemetryManagerInit(void)
* @param    void      @brief void
* @return   esp_err_t @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t TelemetryManagerInit(void)
{
    portENTER_CRITICAL(&s_telemetryMux);

    xTelemetryData.fMosfetTemperature = 0.0f;
    xTelemetryData.fMcuTemperature = 0.0f;
    xTelemetryData.fLineFrequency = 0.0f;

    portEXIT_CRITICAL(&s_telemetryMux);

    return ESP_OK;
}

/*********************************************************************************************************
* @brief    Função chamadas para atualizar as medidas de temperatura
* @fn       void TelemetryManagerUpdateTemperatures(float fMosfetTemperature, float fMcuTemperature)
* @param    fMosfetTemperature      @brief medida da temperatura dos mosfets
* @param    fMcuTemperature      @brief medida da temperatura interna do microcontrolador
* @return   void     @brief void
*********************************************************************************************************/
void TelemetryManagerUpdateTemperatures(float fMosfetTemperature, float fMcuTemperature)
{
    portENTER_CRITICAL(&s_telemetryMux);

    xTelemetryData.fMosfetTemperature = fMosfetTemperature;
    xTelemetryData.fMcuTemperature = fMcuTemperature;

    portEXIT_CRITICAL(&s_telemetryMux);
}

/*********************************************************************************************************
* @brief    Função chamadas para atualizar a medida de frequência da rede
* @fn       void TelemetryManagerUpdateFrequency(float fLineFrequency)
* @param    fLineFrequency      @brief medida da frequência da rede
* @return   void     @brief void
*********************************************************************************************************/
void TelemetryManagerUpdateFrequency(float fLineFrequency)
{
    portENTER_CRITICAL(&s_telemetryMux);

    xTelemetryData.fLineFrequency = fLineFrequency;

    portEXIT_CRITICAL(&s_telemetryMux);
}

/*********************************************************************************************************
* @brief    Função para leitura dos valores de telemetria
* @fn       esp_err_t TelemetryManagerGetData(TELEMETRY_DATA_t *pxData)
* @param    *pxData      @brief ponteiro para a estrutura de medidas calculadas
* @return   esp_err_t    @brief status da estrutura de erros do sistema
*********************************************************************************************************/
esp_err_t TelemetryManagerGetData(TELEMETRY_DATA_t *pxData)
{
    if (pxData == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&s_telemetryMux);

    *pxData = xTelemetryData;

    portEXIT_CRITICAL(&s_telemetryMux);

    return ESP_OK;
}
