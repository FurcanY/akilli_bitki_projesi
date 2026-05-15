#ifndef SYS_CORE_H
#define SYS_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// --- Debug Makroları ---
#define DEBUG_MODE_ENABLED 1

// --- FSM (Durum Makinesi) Sinyalleri ---
//
typedef enum {
    STATE_BOOT,
    STATE_SENSOR_READ,
    STATE_EVALUATE,
    STATE_PUBLISH,
    STATE_DISPLAY,
    STATE_SLEEP,
    STATE_ACTUATE_PUMP,
    STATE_ACTUATE_FAN,
    STATE_TOUCH_WAKEUP,
    STATE_CMD_RECEIVED,
    STATE_CONFIG_UPDATE,
    STATE_FORCE_SENSOR,
    STATE_ERROR
} system_state_t;

// --- Sistem EventGroup Bit Tanımlamaları ---
// Ne yapıyor? Tüm sistemin anlık durumunu tek bir 32-bit register üzerinde tutar.
// Neden yapıyor? Task'ların polling (sürekli kontrol) yapmadan, ilgili bitin değişmesini uykuda beklemesini (block state) sağlar.

#define EVENT_WIFI_CONNECTED    (1 << 0)  // Wi-Fi STA bağlandı ve IP aldı
#define EVENT_WIFI_DISCONNECTED (1 << 1)  // Wi-Fi bağlantısı koptu
#define EVENT_MQTT_CONNECTED    (1 << 2)  // MQTT broker bağlantısı aktif
#define EVENT_SENSOR_DONE       (1 << 3)  // Sensör okuması başarıyla tamamlandı
#define EVENT_SENSOR_FAIL       (1 << 4)  // Sensör okuması başarısız oldu (Timeout/Hata)
#define EVENT_PUMP_ACTIVE       (1 << 5)  // Su pompası şu an çalışıyor (Fan engellemek için)
#define EVENT_FAN_ACTIVE        (1 << 6)  // Fan şu an çalışıyor (Pompayı engellemek için)

// --- Sistem Hazır Maskesi ---
// Ne yapıyor? Ana sistemin dış dünyayla haberleşmeye hazır olup olmadığını tek seferde kontrol eder.
#define SYSTEM_READY_MASK       (EVENT_WIFI_CONNECTED | EVENT_MQTT_CONNECTED)


// --- Veri Paketleri (Structs) ---

// Sensör Veri Paketi
typedef struct {
    float   temperature;    // DHT11 sıcaklık değeri (°C)
    float   humidity;       // DHT11 nem değeri (%)
    int     soil_raw;       // Toprak nemi ham ADC değeri (0-4095)
    int     soil_percent;   // Toprak nemi yüzdelik değeri (0-100)
    bool    sensor_ok;      // Veri güvenilirliği bayrağı
    int64_t timestamp;      // Ölçüm zamanı (mikrosaniye)
} sensor_data_t;

// Aktüatör Komut Paketi (PWM Uyumlu)
// Ne yapıyor? Pompa ve Fan için hız/güç değerlerini taşır.
// Neden yapıyor? PWM ile hız kontrolü sağlamak için (0-1023).
typedef struct {
    uint16_t pump_speed;    // Pompa PWM değeri (0-1023)
    uint16_t fan_speed;     // Fan PWM değeri (0-1023)
} actuator_cmd_t;

// NVS Konfigürasyon Paketi
typedef struct {
    uint32_t sleep_period_s;
    uint32_t pump_dur_s;
    uint32_t fan_dur_s;
    uint32_t pump_cooldown_s;
    uint8_t  soil_threshold;
    uint8_t  temp_threshold;
    uint16_t oled_timeout_s;
    uint16_t mqtt_keepalive;
    char     mqtt_broker[64];
    char     wifi_ssid[32];
    char     wifi_pass[64];
} nvs_config_t;

// --- Global Değişkenler (Extern) ---
extern nvs_config_t     g_cfg;
extern QueueHandle_t    sensor_queue;
extern QueueHandle_t    actuator_queue;      
extern QueueHandle_t    touch_raw_queue;
extern QueueHandle_t    gesture_queue;
extern SemaphoreHandle_t display_semaphore;
extern EventGroupHandle_t sys_events;

// --- Ortak Fonksiyon Prototipleri ---
void sys_core_init(void);
esp_err_t init_nvs_system(void);
esp_err_t load_settings(void);
esp_err_t save_settings(void);
void set_default_settings(void);

#endif // SYS_CORE_H