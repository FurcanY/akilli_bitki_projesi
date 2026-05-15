#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "sys_core.h"
#include "esp_log.h"
#include "sensor_task.h"
#include "actuator_task.h"



void app_main(void)
{
    sys_core_init();

    //sensor_task_init();

    actuator_task_init();

    sensor_data_t received_data; // Kuyruktan gelecek paket

    // test amaçlı aktütaör komutları
    actuator_cmd_t pump_on = { .pump_speed = 1023*40/100,  .fan_speed = 0 };
    actuator_cmd_t fan_on  = { .pump_speed = 0,    .fan_speed = 1023*20/100 };
    actuator_cmd_t all_off = { .pump_speed = 0,    .fan_speed = 0 };   // Hepsi kapalı

    while (1) {
        // burada aktüatör kullanımını test etmek için basit bir döngü oluşturduk.

        // A. Pompayı Başlat
        ESP_LOGI("MAIN_TEST", "Pompa testi baslatiliyor...");
        xQueueSend(actuator_queue, &pump_on, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(750));

        // B. Fanı Başlat
        ESP_LOGI("MAIN_TEST", "Fan testi baslatiliyor. Pompa otomatik durmali.");
        xQueueSend(actuator_queue, &fan_on, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(10000));

        // C. Her şeyi durdur
        ESP_LOGI("MAIN_TEST", "Tum sistem durduruluyor.");
        xQueueSend(actuator_queue, &all_off, portMAX_DELAY);


        vTaskDelay(pdMS_TO_TICKS(10 * 1000)); // 30 saniye bekle ve döngüyü tekrarla
    }
    return;
}
