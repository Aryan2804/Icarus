#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

float lowpass_alpha(float cutoff_hz, float dt)
{
    if (cutoff_hz <= 0.0f) {
        return 1.0f;
    }
    float rc = 1.0f / (2.0f * 3.1415f * cutoff_hz);
    return dt / (dt + rc);
}

struct pid {
    float kp;
    float ki;
    float kd;
    float integrator;
    float integrator_max;
    float prev_error;
    float output_max;
    float d_filter_hz;
    float filtered_derivative;
};

float pid_update(struct pid *pid, float target, float measurement, float dt)
{
    float error = target - measurement;
    float p_term = error * pid->kp;

    pid->integrator += error * pid->ki * dt;
    if (pid->integrator > pid->integrator_max) {
        pid->integrator = pid->integrator_max;
    }
    if (pid->integrator < -pid->integrator_max) {
        pid->integrator = -pid->integrator_max;
    }
    float i_term = pid->integrator;

    float raw_derivative = (error - pid->prev_error) / dt;
    float alpha = lowpass_alpha(pid->d_filter_hz, dt);
    pid->filtered_derivative += alpha * (raw_derivative - pid->filtered_derivative);
    float d_term = pid->filtered_derivative * pid->kd;
    pid->prev_error = error;

    float output = p_term + i_term + d_term;
    if (output > pid->output_max) {
        output = pid->output_max;
    }
    if (output < -pid->output_max) {
        output = -pid->output_max;
    }
    return output;
}

void pid_init(struct pid *pid, float kp, float kd, float ki,
              float integrator_max, float output_max, float d_filter_hz)
{
    pid->kp = kp;
    pid->kd = kd;
    pid->ki = ki;
    pid->integrator_max = integrator_max;
    pid->output_max = output_max;
    pid->d_filter_hz = d_filter_hz;

    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
    pid->filtered_derivative = 0.0f;
}

struct pid roll_rate_pid;

void app_main(void) 
{
    pid_init(&roll_rate_pid, 0.5f, 0.001f, 0.02f, 50.0f, 400.0f, 30.0f);
    float target = 100.0f;
    float measurement = 0.0f;
    float dt = 0.004f;

    while(1) {
        float output = pid_update(&roll_rate_pid, target, measurement, dt);

        printf("output = %f\n", output);

        measurement += output * 0.05f; 

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
s