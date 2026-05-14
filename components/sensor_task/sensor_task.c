#include "sensor_task.h"
#include "sys_core.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h" // DHT11 kütüphanesi
#include "driver/adc.h" // ADC sürücüsü

static const char *TAG = "SENSOR_TEST";

// --- Sensör Pin Tanımlamaları --- //
#define GPIO_DHT11_DATA   4    // DHT11 veri pini
#define GPIO_SENSOR_PWR   23   // Sensör grubu güç anahtarı (Q4)

#define GPIO_Q1_SOIL_PWR    26    // Sensör besleme (Q1)
#define ADC_SOIL_CHANNEL    ADC1_CHANNEL_6 // GPIO 34

static void sensor_task_fn(void *pvParameters) {
    sensor_data_t data;        // Veri paketi

    while (1) {
        // ilk olarak DHT11'den sıcaklık ve nem verilerini oku
        read_dht11(&data);
        // ardından toprak nemi oku
        read_soil_moisture(&data);

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

    gpio_reset_pin(GPIO_Q1_SOIL_PWR);
    gpio_set_direction(GPIO_Q1_SOIL_PWR, GPIO_MODE_OUTPUT);

    // ADC Yapılandırması
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_SOIL_CHANNEL, ADC_ATTEN_DB_12);

    // --- Task Oluşturma --- //
    // Öncelik: 4, Core: 1
    xTaskCreatePinnedToCore(sensor_task_fn, "sensor_task", 4096, NULL, 4, NULL, 1);
}

/**
 * @brief DHT11 sensöründen sıcaklık ve nem verilerini okur.
 * 
 * 
 */
void read_dht11(sensor_data_t *data) {
    // Sensör transistörüne güç ver (Q4 ON)
    gpio_set_level(GPIO_SENSOR_PWR, 1);
    vTaskDelay(pdMS_TO_TICKS(1500)); // Stabilizasyon süresi 
    
    esp_err_t res;
    // Veriyi oku
    res = dht_read_float_data(DHT_TYPE_DHT11, GPIO_DHT11_DATA, 
                                &data->humidity, &data->temperature);

    if (res == ESP_OK) {
        data->sensor_ok = true;
        if (SENSOR_DEBUG_ENABLED) {
            ESP_LOGI(TAG, "DHT11 Data: -> T: %.1f, H: %.1f", data->temperature, data->humidity);
        }
    } 
    else {
        data->sensor_ok = false;
        if (SENSOR_DEBUG_ENABLED) {
            ESP_LOGE(TAG, "DHT11 read hata: %d", res);
        }
    }

    // Gücü kes (Q4 OFF)
    gpio_set_level(GPIO_SENSOR_PWR, 0);
}


/**
 * @brief Toprak nem sensöründen ham ADC değerini okuyup, yüzdelik değere çevirir.
 * 
 * 
 */
void read_soil_moisture(sensor_data_t *data){

    int raw_val;
    gpio_set_level(GPIO_Q1_SOIL_PWR, 1);
    vTaskDelay(pdMS_TO_TICKS(200)); // Kısa bir ısınma süresi yeterli

    //Analog değeri oku (0 - 4095)
    raw_val = adc1_get_raw(ADC_SOIL_CHANNEL);
    data->soil_raw = raw_val;

    // Değeri yüzdeye map et
    data->soil_percent = (4095 - raw_val) * 100 / (4095 - 1800);
    
    // Sınırlandırma (Clamping)
    if(data->soil_percent > 100) data->soil_percent = 100;
    if(data->soil_percent < 0) data->soil_percent = 0;

    if ( SENSOR_DEBUG_ENABLED) {
        ESP_LOGI(TAG, "Toprak Nem: %d (Raw: %d)", data->soil_percent, data->soil_raw);
    }

    // Gücü kes (Korozyonu engelle!)
    gpio_set_level(GPIO_Q1_SOIL_PWR, 0);



}