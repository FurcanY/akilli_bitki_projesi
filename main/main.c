#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "sys_core.h"
#include "esp_log.h"
#include "sensor_task.h"



void app_main(void)
{
    sys_core_init();

    sensor_task_init();

    sensor_data_t received_data; // Kuyruktan gelecek paket

    while (1) {
        // Kuyrukta veri olana kadar bloklan (bekle), polling yapma!
        if (xQueueReceive(sensor_queue, &received_data, portMAX_DELAY) == pdPASS) {
            
            ESP_LOGI("MAIN_FSM", "--- Kuyruktan Veri Alindi ---");
            
            if (received_data.sensor_ok) {
                ESP_LOGI("MAIN_FSM", "Sicaklik: %.1f C, Nem: %.1f %%, Toprak Nem: %d%% (Raw: %d)", 
                        received_data.temperature, 
                        received_data.humidity, 
                        received_data.soil_percent,
                        received_data.soil_raw);
            } 
            else {
                ESP_LOGW("MAIN_FSM", "Gelen veri hatali (sensor_ok = false)");
            }
            
            // Burada ileride STATE_EVALUATE fonksiyonu çağrılacak
        }
    }

    // bütün task'ler başlatıldıktan sonra ana döngü return edilebilir.
    // şu an test için while döngüsünde queue okunuyor.
    return;
}
