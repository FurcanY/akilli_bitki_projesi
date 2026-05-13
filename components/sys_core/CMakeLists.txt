idf_component_register(SRCS "sys_core.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos
                    nvs_flash
                    )

# FreeRTOS kütüphanelerini (Queue, Mutex) kullanacağımız için gereklidir.
