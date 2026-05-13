#include "sys_core.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "config.h" // WiFi SSID ve şifre gibi gizli bilgileri içerir
#include "nvs.h"

static const char *TAG = "SYS_CORE";

// --- NVS Ayarları ---
#define NVS_NAMESPACE "plant_cfg"

// --- Global Değişkenlerin Fiziksel Tanımları ---
nvs_config_t g_cfg;

QueueHandle_t sensor_queue;
QueueHandle_t cmd_queue;
QueueHandle_t touch_raw_queue;
QueueHandle_t gesture_queue;

SemaphoreHandle_t actuator_mutex;
SemaphoreHandle_t display_semaphore;

EventGroupHandle_t sys_events;

// --- Başlatma Fonksiyonu ---
// Ne yapıyor? Tüm FreeRTOS haberleşme objelerini bellekte ayırır (malloc).
// Neden yapıyor? Task'lar çalışmaya başlamadan önce haberleşme yolları hazır olmalıdır.

/**
 * @file sys_core.h
 * @brief Merkezi sistem olay grubu, veri yapıları, tanımlamaları ve haberleşme altyapısını tanımlar.
 *
 */
void sys_core_init(void) {
    ESP_LOGI(TAG, "Sistem cekirdegi baslatiliyor...");

    // 10 adet sensor_data_t taşıyabilecek kuyruk
    sensor_queue = xQueueCreate(10, sizeof(sensor_data_t)); 
    // 5 adet komut taşıyabilecek kuyruk
    cmd_queue = xQueueCreate(5, sizeof(actuator_cmd_t));
    
    // Dokunmatik sensör için kuyruklar
    touch_raw_queue = xQueueCreate(10, sizeof(uint32_t)); // Sadece tick sayısını taşır
    gesture_queue = xQueueCreate(5, sizeof(uint8_t));

    // Mutex ve Semaphore oluşturma
    actuator_mutex = xSemaphoreCreateMutex();
    display_semaphore = xSemaphoreCreateBinary();
    
    // Başlangıçta ekranın güncellenebilmesi için semaforu serbest bırak
    xSemaphoreGive(display_semaphore); 

    // Event Group oluşturma
    sys_events = xEventGroupCreate();

    if(sensor_queue == NULL || cmd_queue == NULL || actuator_mutex == NULL) {
        ESP_LOGE(TAG, "Kritik Hata: FreeRTOS objeleri olusturulamadi!");
    } else {
        ESP_LOGI(TAG, "FreeRTOS objeleri basariyla olusturuldu.");
    }

    // NVS sistemini başlat
    ESP_ERROR_CHECK(init_nvs_system());
    ESP_ERROR_CHECK(load_settings()); // Açılışta ayarları yükle
    
    ESP_LOGI("SYS_CORE", "NVS ve Ayarlar hazir.");


}


/**
 * @file sys_core.h
 * @brief NVS default ayarlarını yükler. Eğer NVS'de veri yoksa veya okunamazsa, varsayılan değerler atanır ve kaydedilir.
 *
 */
void set_default_settings(void) {
    g_cfg.sleep_period_s = 30;
    g_cfg.pump_dur_s = 5;
    g_cfg.fan_dur_s = 30;
    g_cfg.soil_threshold = 30;
    g_cfg.temp_threshold = 28;
    g_cfg.oled_timeout_s = 30;
    g_cfg.mqtt_keepalive = 60;
    strncpy(g_cfg.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(g_cfg.wifi_ssid));
    strncpy(g_cfg.wifi_pass, DEFAULT_WIFI_PASS, sizeof(g_cfg.wifi_pass));
    // Not: Broker IP'sini boş bırakabilir veya default bir IP atayabilirsin.
}

/**
 * @file sys_core.h
 * @brief NVS ayarlarını yükler. Eğer NVS'de veri yoksa veya okunamazsa, varsayılan değerler atanır ve kaydedilir.
 *
 */
esp_err_t load_settings(void) {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW("NVS", "Ayarlar bulunamadi, varsayilanlar yukleniyor...");
            set_default_settings();
            save_settings(); // Varsayılanları kalıcı yap
            return ESP_OK;
        }
        return err;
    }

    // Struct'ı tek blok halinde okuma (Blob işlemi)
    size_t size = sizeof(nvs_config_t);
    err = nvs_get_blob(handle, "config_struct", &g_cfg, &size);
    
    nvs_close(handle);
    return err;
}

/**
 * @file sys_core.h
 * @brief NVS içindeki g_cfg yapısını tek blok halinde kaydeder. Değişiklikler fiziksel flash'a yazılır.
 *
 */
esp_err_t save_settings(void) {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, "config_struct", &g_cfg, sizeof(nvs_config_t));
    if (err == ESP_OK) {
        err = nvs_commit(handle); // Değişiklikleri fiziksel flash'a yaz
    }

    nvs_close(handle);
    return err;
}

/**
 * @file sys_core.h
 * @brief NVS sistemini başlatır. 
 *
 */
esp_err_t init_nvs_system(void) {
    esp_err_t ret = nvs_flash_init();
    // Eğer flash doluysa veya yeni bir versiyon geldiyse sil ve tekrar dene
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}