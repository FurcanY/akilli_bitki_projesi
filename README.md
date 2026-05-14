# Akıllı Bitki Projesi

Bu projeyi elimde var olan bitkimde kullanmak için oluşturuyorum. 

Amacım bitkimin verilerinin bilgisini tutmak ve bu bilgilere göre bitkinin ihtiyaçlarına otomatik cevap vermek.

## Yapılacaklar

- [x] Bağlantı Kurulumu ve core component oluşturulması. Temel yapının test edilmesi.

    - [x] Git reposunun başlatılması ve idf.py create-project ile ana projenin oluşturulması.

    - [x] sys_core component'inin ve CMakeLists.txt dosyasının oluşturulması.

    - [x] sys_core.h içerisine global struct yapılarının (sensor_data_t, actuator_cmd_t, history_data_t) ve FSM durum enumlarının tanımlanması.

    - [x] Task'lar arası iletişimi sağlayacak FreeRTOS kuyruklarının (cmd_queue, sensor_queue vb.) ve sys_events EventGroup'unun oluşturulması (init).

    - [x] Kalıcı hafıza (NVS) başlatılması ve eğer boşsa g_cfg struct'ının varsayılan değerlerle (örn: 30sn uyku, 5sn sulama) yüklenmesi.

- [x] Sensör Katmanının kurulması. Her bir sensörün kullanımının doğrulanması.

    - [x] task_sensor component'inin oluşturulması ve REQUIRES sys_core tanımının yapılması.

    - [x] DHT11 kütüphanesinin eklenmesi ve GPIO 4 üzerinden sıcaklık/nem okunması.

    - [x] Toprak nem sensörü için ADC1_CH6 (GPIO 34) yapılandırmasının yapılması ve ham değerin %0-100 aralığına map edilmesi.

    - [x] Korozyon önlemi algoritmasının yazılması: Sensör okumadan önce GPIO 26 HIGH, 200ms bekleme, okuma ve ardından LOW.

    - [x] Okunan güvenilir verilerin sensor_data_t paketine doldurulup sensor_queue'ya iletilmesi.

- [ ] Aktüatör Katmanının kurulması. Kolektif verilere göre sistemin cevap vermesinin sağlanması.

    - [ ] task_actuator component'inin oluşturulması.

    - [ ] Pompa rölesi (GPIO 27) ve fan motoru (GPIO 25) çıkışlarının yapılandırılması.

    - [ ] cmd_queue üzerinden gelen komutları (örn: CMD_PUMP, CMD_ON) çözümleyen mantığın yazılması.

    - [ ] Donanım güvenliği: actuator_mutex kullanılarak pompa ve fanın aynı anda çalışmasını engelleyen (mutual exclusion) kilit sisteminin kurulması.

    - [ ] Aşırı sulamayı önlemek için pump_cooldown_timer (örn: 30dk) süresince pompanın tekrar çalışmasını reddeden mantığın eklenmesi.

- [ ] FSM ve Ana kontrol sisteminin entegre edilmesi.

    - [ ] main/main.c içinde main_task fonksiyonunun oluşturulması ve switch-case current_state döngüsünün kurulması.

    - [ ] Tüm task'ların (Sensor, Actuator vb.) xTaskCreatePinnedToCore ile doğru çekirdeklerde ve önceliklerde başlatılması.

    - [ ] do_evaluate() fonksiyonunun yazılması: Gelen sensör verilerinin g_cfg.soil_threshold ve temp_threshold ile kıyaslanıp otomatik CMD_PUMP veya CMD_FAN komutlarının üretilmesi.

- [ ] WiFi özelliğinin eklenmesi.

    - [ ] task_wifi component'inin (Event Handler) oluşturulması.

    - [ ] NVS üzerinden wifi_ssid ve wifi_pass bilgilerinin okunarak ESP32'nin Station (STA) modunda ağa bağlanması.

    - [ ] Bağlantı başarılı olduğunda sys_events içindeki EVENT_WIFI_CONNECTED bitinin 1 yapılması.

    - [ ] Kopma durumları için otomatik yeniden bağlanma (reconnect) mantığının kurgulanması.

- [ ] MQTT ile dış dünyaya veri gönderimi.

    - [ ] task_mqtt component'inin eklenmesi ve cJSON parser entegrasyonu.

    - [ ] Broker'a bağlanma ve plant/status/online topic'ine LWT (Last Will) olarak "offline" bilgisinin ayarlanması.

    - [ ] FSM'nin STATE_PUBLISH adımında sensor_data_t paketinin JSON'a çevrilip plant/sensors topic'ine gönderilmesi.

    - [ ] Mobil uygulamadan gelen komutlar için plant/cmd/+ topic'ine abone olunması ve gelen JSON komutların parse edilip cmd_queue'ya aktarılması.

- [ ] OLED ekran, touch sensörün eklenmesi. Manuel olarak sistemin kontrolünü sağlamak.

    - [ ] task_touch modülünde GPIO 14 pinine Kesme (ISR) tanımlanması ve touch_raw_queue'ya sinyal basılması.

    - [ ] Gelen ISR sinyalinin süresini ölçen debounce mantığıyla Tek Basma, Çift Basma ve Uzun Basma gesture'larının ayrıştırılıp gesture_queue'ya aktarılması.

    - [ ] task_display component'i içinde SSD1306 (I2C) ekran sürücüsünün başlatılması.

    - [ ] OLED menü sayfalarının çizilmesi ve touch'tan gelen GESTURE_SINGLE_TAP sinyaliyle sayfalar arası geçişin sağlanması.

    - [ ] Ekranın açık kalma süresi (g_cfg.oled_timeout_s) bittiğinde ekranı kapatan timer fonksiyonunun eklenmesi.

- [ ] ESP32 deepsleep sisteminin entegre edilmesi.

    - [ ] FSM döngüsünde STATE_SLEEP durumuna girildiğinde esp_light_sleep_start() fonksiyonunun tetiklenmesi.

    - [ ] Light sleep boyunca Wi-Fi bağlantısını aktif tutacak mqtt_keepalive (örn: 60sn) konfigürasyonunun yapılması.

    - [ ] Sistem uykudayken ölçüm için uyanmasını sağlayacak periyodik RTC timer kaynağının (g_cfg.sleep_period_s) kurulması.

    - [ ] Ekran ve sistem kontrolü için EXT0 (GPIO 14) pini ile dokunmatik sensörün dış uyandırma (external wakeup) kaynağı olarak ayarlanması.