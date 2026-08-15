# Integration notes

Drop these into an ESP-IDF `main/` component (all four `.c`/`.h` files in the
same folder, listed in your `CMakeLists.txt` `SRCS`). There is exactly one
`app_main()` now, in `flight_controller_main.c`.

## Files
- `bmi088.h` / `bmi088.c` — SPI driver for the accelerometer + gyro.
- `pid.h` / `pid.c` — reusable PID math (state, gains, gain-blend, update, clamp).
- `flight_controller_main.c` — `app_main`: peripheral init, sensor fusion,
  PID loop, motor mixing, arm/disarm safety.

## Bugs fixed while merging
1. `spi_bus_initialize(SPI2_HOST, &cfg, ...)` referenced the wrong variable
   name (`cfg` vs the struct actually named `cnfg`) — wouldn't compile.
2. `gyro_read` / `accel_read` had no return type (implicit `int`, and one
   call site used `aaccel_spi`, a typo for `accel_spi`) — wouldn't compile,
   and would've grabbed the wrong SPI device even if it had.
3. Missing semicolons after `tx[0] = reg_addr | 0x80`.
4. No `TAG` defined for the `ESP_LOGI` calls — wouldn't compile.
5. `app_main` called `bmi088_writel()`, but the function was named
   `bmi088_write()` — typo, wouldn't link.
6. `const int Throttle = {0};` in the motor-mixing file, then later
   reassigned inside `flight_path()` — can't assign to a `const`, wouldn't
   compile. It's now a regular `float` local to the main loop.
7. **Three separate `app_main()` definitions** across the three files — only
   one is allowed per project. Consolidated into one control loop.
8. Unit mismatch: `tcfg.launch_end` / `flight_start` (1350/1500) are clearly
   RC-style microsecond values, but `pwm_set_speed()` wants a 0–100% duty
   cycle, and `Throttle` was undefined units. I standardized on
   microseconds (1000–2000) everywhere throttle/roll/pitch numbers get
   added together, and convert to duty % only at the very last step
   (`us_to_duty_percent`), right before writing to the LEDC peripheral.
9. `speeds[MOTORS]` was declared `int` but fed `float` PID output and then
   passed to a function expecting a `float duty_percent` — silently
   truncated all the sub-1.0 precision. Now kept as `float` end-to-end and
   explicitly clamped 0–100 in `pwm_set_speed`.
10. `uart_read_int()` blocked for up to 50ms per keystroke inside a loop
    meant to run at 1kHz — would have stalled the control loop badly if
    called there. Replaced with a **non-blocking** single-byte poll used
    only for an arm ('a') / disarm ('d') command.

## Safety features added (not in your originals — please keep these)
- **Motors are off until armed.** `armed` starts `false`; PWM duty is
  forced to 0 unless you send `a` over UART. This is the single most
  important guard against an accidental spin-up during bench testing.
- **IMU init is checked.** If the chip IDs don't read back correctly,
  `app_main` halts instead of flying with garbage sensor data.
- Mission auto-disarms itself once `STAGE3_END` passes.

## What's still a TODO (design decisions I couldn't make for you)
- **No RC/receiver or telemetry input yet.** `desired_roll`/`desired_pitch`
  are hard-coded to 0° (self-level only) and throttle comes purely from the
  timed `flight_path()` profile. Fine for a tethered/bench stability test;
  not something you'd want to actually fly untethered without a way to
  intervene.
- **`HOVER_THROTTLE_US` (1450) is a guess** — you'll need to find your
  airframe's real hover throttle empirically (props on, thrust stand or
  very cautious tethered test) and update it.
- **Confirm your motor drive electronics match `pwm_set_speed`.** This code
  drives GPIOs with a straight 4kHz duty-cycle PWM via LEDC. That's correct
  if you're driving motors through simple FETs (brushed motors) or ESCs
  that specifically support duty-cycle/DShot-style input. Standard analog
  RC ESCs expect a 1000–2000µs pulse at ~50Hz (servo-style), *not* a duty
  cycle — if that's your hardware, you'd want the `mcpwm` or `ledc` set up
  to generate that pulse shape instead, using `mix_us[i]` directly rather
  than converting to a percentage. Tell me which ESCs/motors you're using
  and I can adjust this.
- **PID gains, `max_output`, and the launch/flight throttle thresholds are
  all unverified** — they came from your original files and haven't been
  tuned or tested on real hardware.

## Before spinning motors
Remove propellers for all bench testing. Verify motor mixing directions
(each motor spins the correct way and is in the correct mixer slot) with
props off and the frame secured before ever arming with props on.
