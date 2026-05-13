idf_component_register(SRCS "sensor_task.c"
                    INCLUDE_DIRS "include"
                    REQUIRES sys_core driver 
                    dht
)
# sys_core verileri, driver ise GPIO/ADC için

# dht kütüphanesi harici olarak .yml ile dedpendency olarak eklenmiştir
#idf.py add-dependency "esp-idf-lib/dht"
