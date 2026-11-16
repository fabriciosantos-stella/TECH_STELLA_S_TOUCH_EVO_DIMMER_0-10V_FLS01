/*
 *  main.c
 *
 *  Created on: Nov 3, 2026
 *      Author: rodrigo.arnhold
 *
 */

// Includes ----------------------------------------------------------------------------------------------
#include "esp_err.h"
#include "esp_log.h"
#include "status_led.h"
#include "storage_manager.h"
#include "uart_manager.h"
#include "temperature_manager.h"
#include "thermal_protection.h"
#include "phase_control.h"

// Private define ----------------------------------------------------------------------------------------
// Private typedef ---------------------------------------------------------------------------------------
// Private enumerations ----------------------------------------------------------------------------------
// Private variables -------------------------------------------------------------------------------------
static const char *TAG = "APP_MAIN";

// Private function prototypes ---------------------------------------------------------------------------
// Private functions -------------------------------------------------------------------------------------
/*********************************************************************************************************
* @brief    Função principal do sistema
* @fn       void app_main(void)
* @param    void        @brief void
* @return   void        @brief void
*********************************************************************************************************/
void app_main(void)
{
    esp_err_t xError;

    ESP_LOGI(TAG, "Initializing application");
    xError = TemperatureManagerInit(); // Inicializa aquisição das temperaturas

    if (xError != ESP_OK)
    {
        ESP_LOGE("APP_MAIN", "TemperatureManager initialization failed");
        return;
    }
    
    xError = StatusLedInit(); // Inicializa LED de status

    if (xError != ESP_OK)
    {
        ESP_LOGE("APP_MAIN", "StatusLed initialization failed");
        return;
    }

    xError = ThermalProtectionInit(); // Inicializa proteção térmica

    if (xError != ESP_OK)
    {
        ESP_LOGE("APP_MAIN", "ThermalProtection initialization failed");
        return;
    }

    xError = PhaseControlInit(); // Inicializa o controle de corte de fase

    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "Phase Control initialization failed: %s", esp_err_to_name(xError));
        return;
    }

    xError = StorageManagerInit(); // inicializa o gerenciador de parametros do dispositivo

    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "Storage Manager initialization failed: %s", esp_err_to_name(xError));
        StatusLedSetMode(STATUS_LED_SLAVE_COMM_ERROR);        
        return;
    }
 
    xError = UartManagerInit(); // inicializa a interface uart

    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "UART Manager initialization failed: %s", esp_err_to_name(xError));
        StatusLedSetMode(STATUS_LED_SLAVE_COMM_ERROR);
        return;
    }

    xError = StorageManagerSyncWithSlave(); // enfileira o envio dos parametros retentivos para envio para o escravo

    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "Initial parameter synchronization failed");
        return;
    }

    xError = UartManagerStart(); // inicia a tarefa de comunicação entre mestre e escravo pela uart

    if (xError != ESP_OK)
    {
        ESP_LOGE(TAG, "UART Manager task initialization failed");
        return;
    }
    
    ESP_LOGI(TAG, "Application initialized successfully");
}
