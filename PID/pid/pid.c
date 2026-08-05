#include "pid.h"

void PID_Init(PID_HandleTypeDef *pid,
              float Kp, float Ki, float Kd,
              float setpoint,
              float output_min, float output_max,
              float sample_time_s)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->setpoint      = setpoint;
    pid->sample_time_s = sample_time_s;

    pid->output_min = output_min;
    pid->output_max = output_max;

    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Compute(PID_HandleTypeDef *pid, float measurement)
{
    float error, derivative, output;
    float integral_limit;

    error = pid->setpoint - measurement;

    /* Tich phan */
    pid->integral += error * pid->sample_time_s;

    /* Chong tich phan bao hoa (anti-windup): gioi han integral term
       sao cho Ki * integral khong vuot qua output_max/min */
    if (pid->Ki > 0.0001f || pid->Ki < -0.0001f) {
        integral_limit = pid->output_max / pid->Ki;
        if (integral_limit < 0.0f) integral_limit = -integral_limit;

        if (pid->integral > integral_limit)  pid->integral = integral_limit;
        if (pid->integral < -integral_limit) pid->integral = -integral_limit;
    }

    /* Vi phan */
    derivative = (error - pid->prev_error) / pid->sample_time_s;

    output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

    /* Gioi han output */
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;

    pid->prev_error = error;

    return output;
}

void PID_Reset(PID_HandleTypeDef *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

void PID_SetTunings(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

void PID_SetSetpoint(PID_HandleTypeDef *pid, float setpoint)
{
    pid->setpoint = setpoint;
}
