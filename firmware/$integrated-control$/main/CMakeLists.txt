idf_component_register(
    SRCS
        "flight_controller_main.c"
        "pid.c"
        "bmi088.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver      # gpio, ledc, spi_master, uart
        esp_timer   # esp_timer_get_time
        log         # esp_log
)
