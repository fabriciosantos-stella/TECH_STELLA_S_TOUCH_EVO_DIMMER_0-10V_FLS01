/*
 *  main.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware_config.h"
#include "status_led.h"
#include "storage_manager.h"
#include "uart_manager.h"
#include "output_control.h"

static const char *TAG = "APP_MAIN";

static void DimmerTask(void *pvParameters)
{
    (void)pvParameters;
    DEVICE_PARAMETERS_t xParams;
    static uint8_t u8LastPercent = 0xFF;
    static uint8_t u8LastRelayState = 0xFF;
    static uint8_t u8LastPhaseMode = 0xFF;


    while (1)
    {

        if (UartManagerGetParameters(&xParams))
        {

            static uint8_t u8LastState = 0xFF;
            static uint8_t u8LastDim = 0xFF;
            static uint8_t u8LastPhase = 0xFF;

            if ((xParams.u8DeviceOutputState != u8LastState) ||
                (xParams.u8DimLevel != u8LastDim) ||
                (xParams.u8PhaseCutMode != u8LastPhase))
            {
                ESP_LOGI(TAG, "Params: state=%u dim=%u min=%u max=%u phase=%u",
                xParams.u8DeviceOutputState,
                xParams.u8DimLevel,
                xParams.u8MinDimLevel,
                xParams.u8MaxDimLevel,
                xParams.u8PhaseCutMode);

                u8LastState = xParams.u8DeviceOutputState;
                u8LastDim = xParams.u8DimLevel;
                u8LastPhase = xParams.u8PhaseCutMode;
            }


            uint8_t u8Percent = 0;

            if (xParams.u8DeviceOutputState != 0)
            {
                // Slider 1..11 -> 10..100%
                u8Percent = (((xParams.u8DimLevel - 1) * 99) / 10) + 1;

                if (u8Percent > 100)
                {
                    u8Percent = 100;
                }
            }

            OutputControlSetPercent(u8Percent);

            if (u8Percent != u8LastPercent)
            {
                ESP_LOGI(TAG, "Dimmer: %u%% (raw=%u)", u8Percent, xParams.u8DimLevel);
                u8LastPercent = u8Percent;
            }

            
            uint8_t u8RelayState = (xParams.u8DeviceOutputState == 0) ? 1 : 0;

            if ((u8RelayState != u8LastRelayState) ||
                (xParams.u8PhaseCutMode != u8LastPhaseMode))
            {
                if (xParams.u8PhaseCutMode == 0)
                {
                    OutputControlSetSK1(u8RelayState);
                    OutputControlSetDimmerMode(0);
                }
                else
                {
                    OutputControlSetDimmerMode(!u8RelayState);
                    OutputControlSetSK1(0);
                }

                u8LastRelayState = u8RelayState;
                u8LastPhaseMode = xParams.u8PhaseCutMode;
            }


        }
        else
        {
            ESP_LOGW(TAG, "Falha ao ler parametros do escravo");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    esp_err_t xError;

    ESP_LOGI(TAG, "Inicializando aplicacao");

    xError = StatusLedInit();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "StatusLed init falhou: %s", esp_err_to_name(xError));
        return;
    }

    xError = OutputControlInit();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "OutputControl init falhou: %s", esp_err_to_name(xError));
        return;
    }
    OutputControlSetSK1(0);

    OutputControlSetDimmerMode(0);


    xError = StorageManagerInit();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "StorageManager init falhou: %s", esp_err_to_name(xError));
        StatusLedSetMode(STATUS_LED_SLAVE_COMM_ERROR);
        return;
    }

    xError = UartManagerInit();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "UART Manager init falhou: %s", esp_err_to_name(xError));
        StatusLedSetMode(STATUS_LED_SLAVE_COMM_ERROR);
        return;
    }

    xError = StorageManagerSyncWithSlave();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "Sincronizacao inicial falhou: %s", esp_err_to_name(xError));
        return;
    }

    xError = UartManagerStart();
    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "UART Manager start falhou: %s", esp_err_to_name(xError));
        return;
    }

    if (xTaskCreate(DimmerTask, "DimmerTask", 2048, NULL, 3, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Falha ao criar DimmerTask");
        return;
    }

    ESP_LOGI(TAG, "Aplicacao inicializada com sucesso");
}
