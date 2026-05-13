#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "sys_core.h"



void app_main(void)
{
    sys_core_init();

    while (1) {
        // Ana döngü boş, tüm işler task'larda gerçekleşiyor
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 saniye bekle
    }
}
