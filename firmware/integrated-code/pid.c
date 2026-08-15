#include "pid.h"

void pid_init(pid_state *s)
{
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

float clamp(float val, float lo, float hi)
{
    if (val > hi) return hi;
    if (val < lo) return lo;
    return val;
}
