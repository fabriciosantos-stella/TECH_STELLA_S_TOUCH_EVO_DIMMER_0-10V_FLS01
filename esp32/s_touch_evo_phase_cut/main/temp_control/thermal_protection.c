/*
 *  thermal_protection.c
 *
 *  Created on: Nov 12, 2026
 *      Author: rodrigo.arnhold
 *
 

// Includes ----------------------------------------------------------------------------------------------
#include "thermal_protection.h"
#include "temperature_manager.h"
#include "status_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

// Private define ----------------------------------------------------------------------------------------
#define THERMAL_PROTECTION_TASK_STACK_SIZE       3072
#define THERMAL_PROTECTION_TASK_PRIORITY         3
#define THERMAL_PROTECTION_PERIOD_MS             500
#define THERMAL_PROTECTION_MAX_TEMPERATURE       95.0f
#define THERMAL_PROTECTION_RESET_TEMPERATURE     50.0f

// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static THERMAL_PROTECTION_STATE_t eProtectionState = THERMAL_PROTECTION_INITIALIZING;
static portMUX_TYPE ThermalProtectionMux = portMUX_INITIALIZER_UNLOCKED;
static bool bThermalProtectionInitialized = false;

// Private function prototypes ---------------------------------------------------------------------------
static void ThermalProtectionTask(void *pvParameters);
static void ThermalProtectionUpdate(const TEMPERATURE_DATA_t *pxTemperatureData);

// Private functions -------------------------------------------------------------------------------------
*********************************************************************************************************
* @brief    Função que avalia as temperaturas e atualiza o estado da proteção térmica
* @fn       static void ThermalProtectionUpdate(const TEMPERATURE_DATA_t *pxTemperatureData)
* @param    *pxTemperatureData  @brief ponteiro para a estrutura com as leituras de temperatura
* @return   void            @brief void
********************************************************************************************************
static void ThermalProtectionUpdate(const TEMPERATURE_DATA_t *pxTemperatureData)
{
    THERMAL_PROTECTION_STATE_t eCurrentState;
    THERMAL_PROTECTION_STATE_t eNewState;
    portENTER_CRITICAL(&ThermalProtectionMux);
    eCurrentState = eProtectionState;
    portEXIT_CRITICAL(&ThermalProtectionMux);

    eNewState = eCurrentState;

    if ((eCurrentState == THERMAL_PROTECTION_INITIALIZING) && (pxTemperatureData->bNtcValid == false))
    {        
        eNewState = THERMAL_PROTECTION_INITIALIZING; // Aguarda a primeira aquisição válida. A saída permanece bloqueada durante a inicialização
    }
    else if ((pxTemperatureData->bNtcValid == false) || (!isfinite(pxTemperatureData->fNtcTemperature)))
    {
        eNewState = THERMAL_PROTECTION_SENSOR_FAULT;
    }
    else if (pxTemperatureData->fNtcTemperature >= THERMAL_PROTECTION_MAX_TEMPERATURE)
    {
        eNewState = THERMAL_PROTECTION_OVER_TEMPERATURE;
    }
    else if ((eCurrentState == THERMAL_PROTECTION_OVER_TEMPERATURE) || (eCurrentState == THERMAL_PROTECTION_SENSOR_FAULT))
    {
        if (pxTemperatureData->fNtcTemperature <= THERMAL_PROTECTION_RESET_TEMPERATURE)
        {
            eNewState = THERMAL_PROTECTION_NORMAL;
        }
    }
    else
    {
        eNewState = THERMAL_PROTECTION_NORMAL;
    }

    if (eNewState != eCurrentState)
    {
        portENTER_CRITICAL(&ThermalProtectionMux);
        eProtectionState = eNewState;
        portEXIT_CRITICAL(&ThermalProtectionMux);

        if ((eNewState == THERMAL_PROTECTION_OVER_TEMPERATURE) || (eNewState == THERMAL_PROTECTION_SENSOR_FAULT))
        {
            StatusLedSetMode(STATUS_LED_TEMPERATURE_ERROR);
        }
        else if (eNewState == THERMAL_PROTECTION_NORMAL)
        {
            StatusLedSetMode(STATUS_LED_MODE_NORMAL);
        }
        else
        {

        }
    }
}

*********************************************************************************************************
* @brief    Tarefa periódica de monitoramento da proteção térmica
* @fn       static void ThermalProtectionTask(void *pvParameters)
* @param    *pvParameters  @brief ponteiro para a estrutura de parametros da tarefa
* @return   void            @brief void
********************************************************************************************************
static void ThermalProtectionTask(void *pvParameters)
{
    (void)pvParameters;
    TEMPERATURE_DATA_t xTemperatureData;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        if (TemperatureManagerGetData(&xTemperatureData) == ESP_OK)
        {
            ThermalProtectionUpdate(&xTemperatureData);
        }
        else // se houver falha na obtenção dos dados bloqueia a saída
        {            
            portENTER_CRITICAL(&ThermalProtectionMux);

            THERMAL_PROTECTION_STATE_t ePreviousState = eProtectionState;

            eProtectionState = THERMAL_PROTECTION_SENSOR_FAULT;
            portEXIT_CRITICAL(&ThermalProtectionMux);

            if (ePreviousState != THERMAL_PROTECTION_SENSOR_FAULT)
            {
                StatusLedSetMode(STATUS_LED_TEMPERATURE_ERROR);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(THERMAL_PROTECTION_PERIOD_MS));
    }
}

*********************************************************************************************************
* @brief    Função de inicialização do módulo de proteção termica
* @fn       esp_err_t ThermalProtectionInit(void)
* @param    void      @brief void
* @return   esp_err_t      @brief status da estrutura de erros do sistema
********************************************************************************************************
esp_err_t ThermalProtectionInit(void)
{
    if (bThermalProtectionInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t xResult = xTaskCreate(ThermalProtectionTask, "ThermalProtectionTask", THERMAL_PROTECTION_TASK_STACK_SIZE, NULL, THERMAL_PROTECTION_TASK_PRIORITY, NULL);

    if (xResult != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    bThermalProtectionInitialized = true;

    return ESP_OK;
}

*********************************************************************************************************
* @brief    Função que retorna o estado atual da proteção termica
* @fn       esp_err_t ThermalProtectionGetState(THERMAL_PROTECTION_STATE_t *peState)
* @param    *peState      @brief ponteiro para o estado atual de proteção
* @return   esp_err_t      @brief status da estrutura de erros do sistema
********************************************************************************************************
esp_err_t ThermalProtectionGetState(THERMAL_PROTECTION_STATE_t *peState)
{
    if (peState == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!bThermalProtectionInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    portENTER_CRITICAL(&ThermalProtectionMux);
    *peState = eProtectionState;
    portEXIT_CRITICAL(&ThermalProtectionMux);

    return ESP_OK;
}

*********************************************************************************************************
* @brief    Função que informa se a saída está liberada pela proteção térmica
* @fn       esp_err_t ThermalProtectionIsOutputAllowed(bool *pbAllowed)
* @param    *pbAllowed      @brief ponteiro para o status da proteção termica
* @return   esp_err_t      @brief status da estrutura de erros do sistema
********************************************************************************************************
esp_err_t ThermalProtectionIsOutputAllowed(bool *pbAllowed)
{
    if (pbAllowed == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!bThermalProtectionInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    portENTER_CRITICAL(&ThermalProtectionMux);
    *pbAllowed = (eProtectionState == THERMAL_PROTECTION_NORMAL);
    portEXIT_CRITICAL(&ThermalProtectionMux);

    return ESP_OK;
}
*/