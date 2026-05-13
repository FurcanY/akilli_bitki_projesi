#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "sys_core.h"
#include "sensor_task.h"



void app_main(void)
{
    sys_core_init();


    sensor_task_init();

    // bütün task'ler başlatıldıktan sonra ana döngü return edilebilir.
    return;
}
