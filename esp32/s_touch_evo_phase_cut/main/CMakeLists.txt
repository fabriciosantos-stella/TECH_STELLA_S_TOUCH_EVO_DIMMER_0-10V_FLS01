idf_component_register(SRCS "main.c"
                            "output_control/output_control.c"
                            "output_control/phase_control.c"
                            "slider_interface/uart_manager.c"
                            "slider_interface/uart_protocol.c"
                            "status_led/status_led.c"
                            "storage/storage_manager.c"
                            "temp_control/temperature_manager.c"
                            "temp_control/thermal_protection.c"
                            "telemetry/telemetry_manager.c"

                       INCLUDE_DIRS
                            "."
                            "hardware_config"
                            "output_control"
                            "slider_interface"
                            "status_led"
                            "storage"
                            "temp_control"
                            "telemetry"

                       REQUIRES
                            nvs_flash
                            esp_adc
                            esp_driver_tsens
                            esp_driver_gpio
                            esp_driver_uart
                            esp_timer                            
)
