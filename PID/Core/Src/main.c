/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile  uint16_t timer1_flag = 0;
volatile  uint16_t timer1_counter = 0;

void SetTimer(uint16_t duration){
	timer1_flag = 0;
	timer1_counter = duration;
}

void RunTimer(){
	if(timer1_counter > 0){
		timer1_counter --;
		if (timer1_counter == 0){
			timer1_flag = 1;
		}
	}
}

volatile uint8_t mpu_update_flag = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	RunTimer();
	if (htim->Instance == TIM2)
	{
			mpu_update_flag = 1;
	}
}

void Servo_Init()
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Servo_Write(uint8_t angle)
{
	// ccr = (max-min) * angle / 180 + min
	// servo sg90: min 0.5micros, max 2500micros
	uint16_t pulse = (((2500 - 500) * angle) / 180) + 500;
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pulse);
}

#define 	MPU6050_ADDR		(0x68 << 1)
#define 	RAD_TO_DEG 			57.295779513082320876798154814105f
#define 	ALPHA 					0.96f

float SensitivityA 	= 16384.0f;
float SensitivityG	= 131.0f;

float gx, gy, gz;
float ax, ay, az;

float pitch = 0.0f;
float roll = 0.0f;
float yaw = 0.0f;

uint32_t prev_time = 0;

float servo_pitch_angle = 0.0f;

void MPU6050_Init();
void MPU6050_ReadAccel();
void MPU6050_ReadGyro();
void MPU6050_ReadAngle();
void MPU6050_CalculateAngle(float ax, float ay, float az, float gx, float gy, float gz);

void MPU6050_Init(){
	uint8_t check, data;
	
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x75, 1, &check, 1, 1000);
	
	if(check == 0x68){
		data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x6B, 1, &data, 1, 1000);
		data = 0x07;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x19, 1, &data, 1, 1000);
		data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1B, 1, &data, 1, 1000);
		data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1C, 1, &data, 1, 1000);
		data = 0x03;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1A, 1, &data, 1, 1000);
	}
	
}

void MPU6050_ReadAccel(){
	uint8_t data[6];	
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, 1, data, 6, 1000);
	
	int16_t raw_ax = (int16_t)(data[0] << 8 | data[1]);
  int16_t raw_ay = (int16_t)(data[2] << 8 | data[3]);
  int16_t raw_az = (int16_t)(data[4] << 8 | data[5]);

  ax = (float)raw_ax / SensitivityA;
  ay = (float)raw_ay / SensitivityA;
  az = (float)raw_az / SensitivityA;
}

void MPU6050_ReadGyro(){
	uint8_t data[6];	
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x43, 1, data, 6, 1000);
	
	int16_t raw_gx = (int16_t)(data[0] << 8 | data[1]);
  int16_t raw_gy = (int16_t)(data[2] << 8 | data[3]);
  int16_t raw_gz = (int16_t)(data[4] << 8 | data[5]);

  gx = (float)raw_gx / SensitivityG;
  gy = (float)raw_gy / SensitivityG;
  gz = (float)raw_gz / SensitivityG;
}

void MPU6050_CalculateAngle(float ax, float ay, float az, float gx, float gy, float gz)
{
	uint32_t current_time = HAL_GetTick();
	
	if (prev_time == 0) {
		prev_time = current_time;
		return;
	}

	float dt = (float)(current_time - prev_time) / 1000.0f;
	prev_time = current_time;

	if (dt <= 0.0f) return;

	float roll_accel  = atan2f(ay, az) * RAD_TO_DEG;
	float pitch_accel = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

	roll  = ALPHA * (roll  + gx * dt) + (1.0f - ALPHA) * roll_accel;
	pitch = ALPHA * (pitch + gy * dt) + (1.0f - ALPHA) * pitch_accel;

	yaw += gz * dt;
	
}
	
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	MPU6050_Init();
	Servo_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
	//uint8_t duration = 2;
	//SetTimer(duration);
  while (1)
  {
		/*
		if(timer1_flag == 1){		
			SetTimer(duration);
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		}
		*/
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		MPU6050_ReadAccel();
		MPU6050_ReadGyro();
		MPU6050_CalculateAngle(ax, ay, az, gx, gy, gz);
		HAL_Delay(100);
		if (mpu_update_flag)
    {
			mpu_update_flag = 0;

			
			servo_pitch_angle = 90.0f - pitch;

			if (servo_pitch_angle > 180.0f) servo_pitch_angle = 180.0f;
			if (servo_pitch_angle < 0.0f) servo_pitch_angle = 0.0f;

			Servo_Write((uint8_t)servo_pitch_angle);
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
