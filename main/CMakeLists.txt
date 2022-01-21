idf_component_register(SRCS "app_main.c" "timer_driver.c" "gpio_driver.c" "ble_driver.c"
                       REQUIRES "bt" # required to be able to #include esp_bt.h
                       INCLUDE_DIRS ".")
