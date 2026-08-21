idf_component_register(
    SRCS "speed_control.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_driver_usb_serial_jtag vfs
)