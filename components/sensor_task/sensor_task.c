#include "sensor_task.h"
#include "sys_core.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h" // DHT11 kütüphanesi

static const char *TAG = "SENSOR_TEST";

// --- Sensör Pin Tanımlamaları --- //
#define GPIO_DHT11_DATA   4    // DHT11 veri pini
#define GPIO_SENSOR_PWR   23   // Sensör grubu güç anahtarı (Q4)

static void sensor_task_fn(void *pvParameters) {
    sensor_data_t data;        // Veri paketi
    esp_err_t res;

    while (1) {
        // 1. Sensör grubuna güç ver (Q4 ON)
        gpio_set_level(GPIO_SENSOR_PWR, 1);
        vTaskDelay(pdMS_TO_TICKS(1500)); // Stabilizasyon süresi 

        // 2. Veriyi oku
        res = dht_read_float_data(DHT_TYPE_DHT11, GPIO_DHT11_DATA, 
                                 &data.humidity, &data.temperature);

        if (res == ESP_OK) {
            data.sensor_ok = true;
            if (SENSOR_DEBUG_ENABLED) {
                ESP_LOGI(TAG, "DHT11 Data: -> T: %.1f, H: %.1f", data.temperature, data.humidity);
            }
        } else {
            data.sensor_ok = false;
            if (SENSOR_DEBUG_ENABLED) {
                ESP_LOGE(TAG, "DHT11 read hata: %d", res);
            }
        }

        // 3. Gücü kes (Q4 OFF)
        gpio_set_level(GPIO_SENSOR_PWR, 0);

        // 4. Veriyi kuyruğa gönder (Ana sistemle haberleşme)
        if (sensor_queue != NULL) {
            xQueueSend(sensor_queue, &data, 0);
        }

        // 5. Okuma bittiğine dair bayrağı kaldır
        xEventGroupSetBits(sys_events, EVENT_SENSOR_DONE);

        // Bir sonraki ölçüm için NVS'ten gelen süreyi bekle
        vTaskDelay(pdMS_TO_TICKS(g_cfg.sleep_period_s * 1000)); // default 30 saniye
    }
}

/**
 * @brief Sensör task'ını ve gerekli donanım sürücülerini (ADC/GPIO) başlatır.
 * * Bu fonksiyon app_main() içerisinden bir kez çağrılmalıdır.
 * İçerisinde GPIO 26 (Power Switch) ve ADC yapılandırmalarını gerçekleştirir.
 */
void sensor_task_init(void) {
    // --- GPIO Kurulumu --- //
    gpio_reset_pin(GPIO_SENSOR_PWR);
    gpio_set_direction(GPIO_SENSOR_PWR, GPIO_MODE_OUTPUT);

    // --- Task Oluşturma --- //
    // Öncelik: 4, Core: 1
    xTaskCreatePinnedToCore(sensor_task_fn, "sensor_task", 4096, NULL, 4, NULL, 1);
}