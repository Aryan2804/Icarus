// // #include <stdio.h>
// // #include <math.h>

// // //input thre gyro value;

// // void app_main(void)
// // { 
// //     //launching time

// //     if ((Throttle < 1350)){
        
// //         kp = 18.0;
// //         ki=0;
// //         kd = 5.0;
        
// //     }
// // if(Flight Mode (Throttle > 1500){
// //             ki= 2;
// //             kp=3;
// //             kd 3;

// //         }

// // if (landing){
// //     ki=2;
// //     kd=0.5;
// //     kp=2;
// // }
// // output to the motorcontroll
// // }

// struct pid_gain {
//      float launch_kp;
//     float launch_kd;
//     float flight_kp;
//     float flight_kd;
//     float ki;            
//     float max_i_output;  
//     float max_output;  

// }

// struct state {
//     float error_sum; //for i term error accumulation


// }
// float throttle = 1200.0f;      // example value, comes from your radio input
// float launch_end = 1350.0f;    // below this = pure launch gains
// float flight_start = 1500.0f;  // above this = pure flight gains

// float gain_blend;
// int enable_i;

// if (throttle < launch_end) 
// {
//     gain_blend = 0.0f;
//     enable_i = 0;
// } 
// else if (throttle > flight_start)
//  {
//     gain_blend = 1.0f;
//     enable_i = 1;
// } 
// else 
// {
//     gain_blend = (throttle - launch_end) / (flight_start - launch_end);
//     enable_i = 0;
// }


#include <stdio.h>
#include <math.h>

typedef struct {
    float launch_kp;
    float launch_kd;
    float flight_kp;
    float flight_kd;
    float ki;
    float max_i_output;
    float max_output;
} pid_gains;

typedef struct {
    float error_sum;
} pid_state;

typedef struct {
    float launch_end;
    float flight_start;
} throttle_config;

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
pid_gains yaw_gains = {
    .launch_kp = 4.0f, .launch_kd = 0.0f,   // kd unused for yaw, but harmless to leave at 0
    .flight_kp = 2.0f, .flight_kd = 0.0f,
    .ki = 0.5f, .max_i_output = 15.0f, .max_output = 100.0f
};

pid_state roll_state;
pid_state pitch_state;
pid_state yaw_state;

void app_main(void)
{
    pid_init(&roll_state); 
    pid_init(&pitch_state);
    pid_init(&yaw_state); 
    while (1) {
      
        //sensor input

        float gain_blend;
        int enable_i;
        pid_compute_gain_blend(&tcfg, throttle, &gain_blend, &enable_i);

        float roll_output = pid_update(&roll_state, &roll_gains,
                                        desired_roll, current_roll, roll_rate,
                                        dt, gain_blend, enable_i);

        float pitch_output = pid_update(&pitch_state, &pitch_gains,
                                         desired_pitch, current_pitch, pitch_rate,
                                         dt, gain_blend, enable_i);

        float yaw_output = pid_update_no_d(&yaw_state, &yaw_gains,
                                            desired_yaw, yaw_rate,
                                            dt, gain_blend, enable_i);
        //send roll_output to your motor mixer
    }
}