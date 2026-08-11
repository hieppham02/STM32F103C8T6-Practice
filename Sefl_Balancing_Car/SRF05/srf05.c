#include "srf05.h"

void SRF05_Init(SRF05_HandleTypeDef *hsrf, 
                GPIO_TypeDef* TrigPort, uint16_t TrigPin, 
                GPIO_TypeDef* EchoPort, uint16_t EchoPin, 
                TIM_HandleTypeDef* hTimer){
	hsrf->Trig_Port = TrigPort;									
	hsrf->Trig_Pin = TrigPin;
	hsrf->Echo_Port = EchoPort;
	hsrf->Echo_Pin = EchoPin;
	hsrf->hTimer = hTimer;
	hsrf->timeout = 30000;
}

void SRF05_Trigger(SRF05_HandleTypeDef *hsrf){
	HAL_GPIO_WritePin(hsrf->Trig_Port, hsrf->Trig_Pin, 0);
	Delay_us(hsrf->hTimer, 2);
	HAL_GPIO_WritePin(hsrf->Trig_Port, hsrf->Trig_Pin, 1);
	Delay_us(hsrf->hTimer, 10);
	HAL_GPIO_WritePin(hsrf->Trig_Port, hsrf->Trig_Pin, 0);
	
}

float SRF05_GetDistance(SRF05_HandleTypeDef *hsrf){
	float elapsedTime = 0.0f;
	SRF05_Trigger(hsrf);
	
	while(HAL_GPIO_ReadPin(hsrf->Echo_Port, hsrf->Echo_Pin) == 0) {
		if(--hsrf->timeout == 0) return 0;
  }
	
	 __HAL_TIM_SET_COUNTER(hsrf->hTimer, 0);
	
	while (HAL_GPIO_ReadPin(hsrf->Echo_Port, hsrf->Echo_Pin) == 1) {
		if (__HAL_TIM_GET_COUNTER(hsrf->hTimer) > 30000) return 0;
	}
	
	elapsedTime = __HAL_TIM_GetCounter(hsrf->hTimer);
	
	// V = 34.3 m/s = 34300 cm/s 
	// 1s = 1,000,000 microsecond
	// => V = 0.034 cm/microsecond
	// chia 2 vi tinh 2 quang duong di + ve
	return (float)elapsedTime  * (34300.0f / 1000000.0f) / 2 ;
}

void Delay_us(TIM_HandleTypeDef* htime, uint16_t us){
	__HAL_TIM_SET_COUNTER(htime, 0);
    while (__HAL_TIM_GET_COUNTER(htime) < us);
}