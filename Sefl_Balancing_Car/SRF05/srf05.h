#ifndef _SRF05_H
#define _SRF05_H

#include "main.h"

typedef struct {
    GPIO_TypeDef* Trig_Port;
    uint16_t      Trig_Pin;
    GPIO_TypeDef* Echo_Port;
    uint16_t      Echo_Pin;
    TIM_HandleTypeDef* hTimer;
    uint32_t      timeout;
} SRF05_HandleTypeDef;

void SRF05_Init(SRF05_HandleTypeDef *hsrf, 
                GPIO_TypeDef* TrigPort, uint16_t TrigPin, 
                GPIO_TypeDef* EchoPort, uint16_t EchoPin, 
                TIM_HandleTypeDef* hTimer);

void SRF05_Trigger(SRF05_HandleTypeDef *hsrf);

float SRF05_GetDistance(SRF05_HandleTypeDef *hsrf);

void Delay_us(TIM_HandleTypeDef* hTimer, uint16_t us);

#endif