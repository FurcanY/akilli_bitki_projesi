#include "sensor_task.h"
#include "sys_core.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h" // DHT11 kütüphanesi

static const char *TAG = "SENSOR_TEST";

// --- Sensör Pin Tanımlamaları --- //
#define GPIO_DHT11_DATA   4    // DHT11 veri pini
#define GPIO_SENSOR_PWR   23   // Sensör grubu güç anahtarı (Q1)

// --- Test Fonksiyonu --- //
static void sensor_task_fn(void *pvParameters) {
    sensor_data_t data;
    esp_err_t res;

    while (1) {
        // 1. Sensörlere güç ver
        // Neden? enerji tasarrufu sağlamak için.
        gpio_set_level(GPIO_SENSOR_PWR, 1);
        
        // 2. Sensörün stabilize olması için bekle (1500 ms)
        vTaskDelay(pdMS_TO_TICKS(1500));

        // 3. Veriyi oku
        // Hangi sinyal? DHT_TYPE_DHT11 tipindeki sensörden nem ve sıcaklık.
        res = dht_read_float_data(DHT_TYPE_DHT11, GPIO_DHT11_DATA, 
                                 &data.humidity, &data.temperature);

        if (res == ESP_OK) {
            ESP_LOGI(TAG, "Sıcaklık: %.1f°C, Nem: %.1f%%", data.temperature, data.humidity);
            data.sensor_ok = true;
        } else {
            ESP_LOGE(TAG, "DHT11 okuma hatası! Kod: %d", res);
            data.sensor_ok = false;
        }

        // 4. Ölçüm bitti, gücü kes
        gpio_set_level(GPIO_SENSOR_PWR, 0);

        // 5. Test aşamasında olduğumuz için veriyi sadece logluyoruz.
        // İleride burada xQueueSend(sensor_queue, &data, ...) olacak.
        
        // Ölçümler arası 5 saniye bekle
        vTaskDelay(pdMS_TO_TICKS(5000));
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