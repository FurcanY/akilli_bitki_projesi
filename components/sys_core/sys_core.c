#include "sys_core.h"
#include "esp_log.h"

static const char *TAG = "SYS_CORE";

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
}