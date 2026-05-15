#include "actuator_task.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "ACTUATOR_TASK";

// --- PWM ve Donanım Tanımlamaları --- //
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT    // 0-1023 arası çözünürlük
#define LEDC_FREQUENCY          (5000)               // 5 kHz frekans

// --- Sinyal Kanalları --- //
#define GPIO_Q2_PUMP            27                   // Pompa kontrol sinyali (Q2)
#define GPIO_Q3_FAN             25                   // Fan kontrol sinyali (Q3)

#define LEDC_CHANNEL_PUMP       LEDC_CHANNEL_0
#define LEDC_CHANNEL_FAN        LEDC_CHANNEL_1

// --- Debug Kontrolleri --- //
#define ACTUATOR_DEBUG          1

/**
 * @brief LEDC (PWM) donanımını yapılandırır.
 * Ne yapıyor? Zamanlayıcı ve kanal ayarlarını yapar.
 * Neden yapıyor? Motor hız kontrolünü mümkün kılmak için.
 */
static void configure_pwm(void) {
    // Zamanlayıcı yapılandırması
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // SU motoru kanal yapılandırması
    ledc_channel_config_t pump_ch = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_PUMP,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_Q2_PUMP,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&pump_ch);

    // Fan kanalı yapılandırması
    ledc_channel_config_t fan_ch = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_FAN,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_Q3_FAN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&fan_ch);
}

/**
 * @brief Aktüatör yönetim görevi.
 * Karşılıklı Dışlama: Pompa çalışırken Fan durdurulur, Fan çalışırken Pompa durdurulur.
 * Neden? 1A adaptör kapasitesini aşmamak için.
 */
static void actuator_task_fn(void *pvParameters) {
    actuator_cmd_t cmd;

    while (1) {
        if (xQueueReceive(actuator_queue, &cmd, portMAX_DELAY) == pdPASS) {
            
            // --- Karşılıklı Dışlama (Mutual Exclusion) Mantığı ---
            // Ne yapıyor? Pompa hızı > 0 ise Fan'ı kapatır, Fan hızı > 0 ise Pompa'yı kapatır.
            // Neden yapıyor? 1A adaptör kapasitesini aşmamak için.
            
            if (cmd.pump_speed > 0) {
                // Önce Fan'ı durdur
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, 0);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN);

                // --- Kick-start (Pompa için) ---
                // %100 güç verip 100ms bekleyerek statik sürtünmeyi aşalım
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PUMP, 1023/2); 
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PUMP);
                vTaskDelay(pdMS_TO_TICKS(150)); // 150ms "vuruntu" süresi

                // Hedef hıza çek
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PUMP, cmd.pump_speed);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PUMP);
                
                ESP_LOGI(TAG, "Pompa Kick-start ile baslatildi: %d PWM", cmd.pump_speed);
            } 
            else if (cmd.fan_speed > 0) {
                // Önce Pompa'yı durdur
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PUMP, 0);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PUMP);

                // --- Kick-start (Fan için) ---
                // Fanlar genelde daha ağır kalkar, 200ms tam güç verelim
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, 1023/2); 
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN);
                vTaskDelay(pdMS_TO_TICKS(200)); 

                // Hedef %30 hıza çek
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, cmd.fan_speed);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN);

                ESP_LOGI(TAG, "Fan Kick-start ile baslatildi: %d PWM", cmd.fan_speed);
            }
            else {
                // İkisini de kapat
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PUMP, 0);
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, 0);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PUMP);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN);
            }
        }
    }
}

void actuator_task_init(void) {
    configure_pwm();
    xTaskCreatePinnedToCore(actuator_task_fn, "actuator_task", 4096, NULL, 5, NULL, 1);
}