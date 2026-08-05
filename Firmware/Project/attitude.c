#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "esp_timer.h"
#include "bmi.h" 



void pid_init(pid_state *s) {
    s->error_sum = 0.0f;
}

void pid_compute_gain_blend(const throttle_config *cfg, float throttle,
                             float *gain_blend, int *enable_i)
{
    if (throttle < cfg->launch_end) {
        *gain_blend = 0.0f;
        *enable_i = 0;
    } else if (throttle > cfg->flight_start) {
        *gain_blend = 1.0f;
        *enable_i = 1;
    } else {
        *gain_blend = (throttle - cfg->launch_end) /
                      (cfg->flight_start - cfg->launch_end);
        *enable_i = 0;
    }
}

float pid_update(pid_state *s, const pid_gains *g,
                  float desired_angle, float current_angle, float current_rate,
                  float dt, float gain_blend, int enable_i)
{
    float error = desired_angle - current_angle;

    float kp = (1.0f - gain_blend) * g->launch_kp + gain_blend * g->flight_kp;
    float kd = (1.0f - gain_blend) * g->launch_kd + gain_blend * g->flight_kd;

    float p_term = kp * error;

    float i_term = 0.0f;
    if (enable_i && g->ki > 0.0f) {
        s->error_sum += error * dt;
        float limit = g->max_i_output / g->ki;
        if (s->error_sum > limit)  s->error_sum = limit;
        if (s->error_sum < -limit) s->error_sum = -limit;
        i_term = g->ki * s->error_sum;
    } else {
        s->error_sum = 0.0f;
    }

    float d_term = kd * current_rate;

    float output = p_term + i_term - d_term;
    if (output > g->max_output)  output = g->max_output;
    if (output < -g->max_output) output = -g->max_output;

    return output;
}

float clamp(float val, float lo, float hi) {
    if (val > hi) return hi;
    if (val < lo) return lo;
    return val;
}


throttle_config tcfg = { .launch_end = 1350.0f, .flight_start = 1500.0f };

pid_gains roll_gains = {
    .launch_kp = 18.0f, .launch_kd = 0.5f,
    .flight_kp = 8.0f,  .flight_kd = 0.2f,
    .ki = 1.5f, .max_i_output = 20.0f, .max_output = 100.0f
};
pid_gains pitch_gains = {
    .launch_kp = 18.0f, .launch_kd = 0.5f,
    .flight_kp = 8.0f,  .flight_kd = 0.2f,
    .ki = 1.5f, .max_i_output = 20.0f, .max_output = 100.0f
};

//fixed yaw
#define YAW_OUTPUT_CONSTANT 0.0f

pid_state roll_state;
pid_state pitch_state;

void app_main(void)
{
    pid_init(&roll_state);
    pid_init(&pitch_state);

    while (1) {

       
        static int64_t last_time_us = 0;
        int64_t now_us = esp_timer_get_time();
        float dt = (last_time_us == 0) ? 0.001f : (now_us - last_time_us) / 1e6f;
        last_time_us = now_us;

        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        bmi088_read_accel(&ax_raw, &ay_raw, &az_raw);
        bmi088_read_gyro(&gx_raw, &gy_raw, &gz_raw);

        float gx_dps = (float)gx_raw / 16.384f;
        float gy_dps = (float)gy_raw / 16.384f;
        float gz_dps = (float)gz_raw / 16.384f;

        float ax_g = (float)ax_raw / 5460.0f;
        float ay_g = (float)ay_raw / 5460.0f;
        float az_g = (float)az_raw / 5460.0f;

        float accel_roll  = atan2f(ay_g, az_g) * 180.0f / (float)M_PI;
        float accel_pitch = atan2f(-ax_g, sqrtf(ay_g*ay_g + az_g*az_g)) * 180.0f / (float)M_PI;

        static float current_roll = 0.0f;
        static float current_pitch = 0.0f;

        const float alpha = 0.98f;
        current_roll  = alpha * (current_roll  + gx_dps * dt) + (1.0f - alpha) * accel_roll;
        current_pitch = alpha * (current_pitch + gy_dps * dt) + (1.0f - alpha) * accel_pitch;

        float roll_rate  = gx_dps;
        float pitch_rate = gy_dps;

        float throttle      = 0.0f;  // raw throttle channel, same units as tcfg (e.g. µs pulse width)
        float desired_roll  = 0.0f;  // stick input mapped to a target roll angle (deg)
        float desired_pitch = 0.0f;  // stick input mapped to a target pitch angle (deg)

        // ---- PID ----
        float gain_blend;
        int enable_i;
        pid_compute_gain_blend(&tcfg, throttle, &gain_blend, &enable_i);

        float roll_output = pid_update(&roll_state, &roll_gains,
                                        desired_roll, current_roll, roll_rate,
                                        dt, gain_blend, enable_i);

        float pitch_output = pid_update(&pitch_state, &pitch_gains,
                                         desired_pitch, current_pitch, pitch_rate,
                                         dt, gain_blend, enable_i);

        float yaw_output = YAW_OUTPUT_CONSTANT;  // unused in mixer since yaw is fixed

        

    }
}