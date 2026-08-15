# Flight Controller — ESP-IDF Project

## Layout
```
flight_controller/
├── CMakeLists.txt          # top-level project file
├── sdkconfig.defaults      # tick rate + stack size overrides (see notes)
└── main/
    ├── CMakeLists.txt      # component registration
    ├── flight_controller_main.c
    ├── pid.c / pid.h
    └── bmi088.c / bmi088.h
```

## Build & flash
```bash
# one-time, per terminal session:
. $HOME/esp/esp-idf/export.sh        # or wherever your IDF install lives

cd flight_controller
idf.py set-target esp32              # change if you're on a different chip
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor # adjust port for your OS/board
```

Requires ESP-IDF v5.x on your machine (not included here — install separately
if you haven't: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/).

## Things worth checking before you arm this on a prop-on airframe

1. **Props off for first flash.** First boot after flashing should be with
   propellers removed — confirm the IMU orientation, gyro/accel signs, and
   `armed`/motor-mixing logic behave as expected before anything can spin.

2. **FreeRTOS tick rate.** The control loop uses `vTaskDelay(pdMS_TO_TICKS(1))`
   assuming a 1kHz loop. ESP-IDF's default tick rate is 100Hz, which would
   silently turn that into a 10ms (100Hz) loop instead. `sdkconfig.defaults`
   sets `CONFIG_FREERTOS_HZ=1000` to fix this — just flag it in case you
   already have your own sdkconfig with a different value, since project
   sdkconfig can override defaults.

3. **UART0 is normally the console/log UART.** `flight_controller_main.c`
   calls `uart_driver_install(UART_NUM_0, ...)` to read arm/disarm commands,
   but UART0 is also where `ESP_LOGx` output and the IDF monitor go by
   default. Installing your own driver on it can conflict with the console
   driver ESP-IDF already sets up (or vice versa), causing dropped bytes or
   init errors. If you see issues, either move the arm/disarm command
   interface to a different UART, or disable the ESP-IDF console driver on
   UART0 in menuconfig.

4. **No receiver input yet.** As already flagged in your own TODO, setpoints
   are hardcoded to level flight and throttle follows a fixed, timed ramp
   profile — there's no RC/telemetry link. Fine for a tethered bench test,
   not for free flight.

5. **`HOVER_THROTTLE_US = 1450`** is a placeholder — tune it to your actual
   airframe's hover point before running the timed mission profile, or the
   craft will climb or sink during the "hold" stage.

None of the above required changing your control/sensor logic — just the
build scaffolding it was missing plus these notes.
