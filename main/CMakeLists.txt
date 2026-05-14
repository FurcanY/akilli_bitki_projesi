idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES sys_core
                    freertos
                    sensor_task
                    driver
                    )
