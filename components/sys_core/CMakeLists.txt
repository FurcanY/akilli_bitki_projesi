idf_component_register(SRCS "sys_core.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos)

# FreeRTOS kütüphanelerini (Queue, Mutex) kullanacağımız için gereklidir.
