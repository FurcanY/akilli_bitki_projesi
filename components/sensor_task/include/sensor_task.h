#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "sys_core.h"

// sensör task'ı içerisinde debug mesajlarını açıp kapatmak için makro
#define SENSOR_DEBUG_ENABLED 1
// --- Görev Başlatma Sinyalleri --- //
void sensor_task_init(void);

// --- Yardımcı Fonksiyonlar --- //
void read_dht11(sensor_data_t *data);
void read_soil_moisture(sensor_data_t *data);

#endif // SENSOR_TASK_H