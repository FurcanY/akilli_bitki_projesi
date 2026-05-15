idf_component_register(SRCS "actuator_task.c"
                    INCLUDE_DIRS "include"
                    REQUIRES sys_core driver)

# PWM (LEDC) sürücülerine ve sistem kuyruklarına erişim için.