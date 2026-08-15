#ifndef PID_H
#define PID_H

typedef struct {
    float error_sum;
} pid_state;

typedef struct {
    float launch_kp, launch_kd;
    float flight_kp, flight_kd;
    float ki;
    float max_i_output;
    float max_output;
} pid_gains;

typedef struct {
    float launch_end;
    float flight_start;
} throttle_config;

void  pid_init(pid_state *s);
void  pid_compute_gain_blend(const throttle_config *cfg, float throttle,
                              float *gain_blend, int *enable_i);
float pid_update(pid_state *s, const pid_gains *g,
                  float desired_angle, float current_angle, float current_rate,
                  float dt, float gain_blend, int enable_i);
float clamp(float val, float lo, float hi);

#endif // PID_H
