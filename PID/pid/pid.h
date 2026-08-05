#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PID module dung cho vong dieu khien goc servo phan hoi tu MPU6050
 * Sample time (Ts) phai khop voi chu ky ngat ban dung de goi PID_Compute()
 * (vi du: TIM2 ngat moi 10ms -> Ts = 0.01f)
 */

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float setpoint;      // gia tri muc tieu, vd 90.0f (do)
    float sample_time_s; // chu ky lay mau, vd 0.01f cho vong lap 100Hz

    float integral;
    float prev_error;

    float output_min;    // gioi han duoi cua output (vd: -400.0f)
    float output_max;    // gioi han tren cua output (vd:  400.0f)
} PID_HandleTypeDef;

void  PID_Init(PID_HandleTypeDef *pid,
               float Kp, float Ki, float Kd,
               float setpoint,
               float output_min, float output_max,
               float sample_time_s);

float PID_Compute(PID_HandleTypeDef *pid, float measurement);

void  PID_Reset(PID_HandleTypeDef *pid);

void  PID_SetTunings(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd);

void  PID_SetSetpoint(PID_HandleTypeDef *pid, float setpoint);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
