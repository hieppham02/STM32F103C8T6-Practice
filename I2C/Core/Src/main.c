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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
#include <string.h>
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define 	ADDR 		0x68 << 1
#define 	RAD_TO_DEG	57.2957795131f

int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;

int16_t gx = 0;
int16_t gy = 0;
int16_t gz = 0;

float AX = 0;
float AY = 0;
float AZ = 0;

float GX = 0;
float GY = 0;
float GZ = 0;

float pitch = 0.0f;
float roll = 0.0f;

float lastPitch = 0.0f;
float lastRoll  = 0.0f;

float AX_offset = 0;
float AY_offset = 0;
float AZ_offset = 0;

float GX_offset = 0;
float GY_offset = 0;
float GZ_offset = 0;

uint8_t mData;

char txBuffer[100];

void MPU6050_Init();
void MPU6050_Read_G();
void MPU6050_Read_A();
void MPU6050_Calibrate(void);
void MPU6050_Filter(float AX, float AY, float AZ, float GX, float GY, float GZ);

void MPU6050_Init(){
	uint8_t data;
	uint8_t m;
	HAL_I2C_Mem_Read(&hi2c1, ADDR, 0x75, 1, &data, 1, 1000);
	if(data == 0x68){
		m = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, ADDR, 0x6B, 1, &m, 1, 1000);
		m = 0x03;
    HAL_I2C_Mem_Write(&hi2c1, ADDR, 0x1A, 1, &m, 1, 1000);
		m = 0x07;
		HAL_I2C_Mem_Write(&hi2c1, ADDR, 0x19, 1, &m, 1, 1000);
		m = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, ADDR, 0x1C, 1, &m, 1, 1000);
		m = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, ADDR, 0x1B, 1, &m, 1, 1000);
	}
}

void MPU6050_Read_G(void)
{
    uint8_t gData[6];
	
    if(HAL_I2C_Mem_Read(&hi2c1, ADDR, 0x43, 1, gData, 6, 1000) != HAL_OK) return;
	
    gx = (int16_t)((gData[0] << 8) | gData[1]);
    gy = (int16_t)((gData[2] << 8) | gData[3]);
    gz = (int16_t)((gData[4] << 8) | gData[5]);

    GX = gx / 131.0f;
    GY = gy / 131.0f;
    GZ = gz / 131.0f;
		
		GX -= GX_offset;
		GY -= GY_offset;
		GZ -= GZ_offset;
}

void MPU6050_Read_A(void)
{
    uint8_t aData[6];

    if(HAL_I2C_Mem_Read(&hi2c1, ADDR, 0x3B, 1, aData, 6, 1000) != HAL_OK) return;

    ax = (int16_t)((aData[0] << 8) | aData[1]);
    ay = (int16_t)((aData[2] << 8) | aData[3]);
    az = (int16_t)((aData[4] << 8) | aData[5]);

    AX = ax / 16384.0f;
    AY = ay / 16384.0f;
    AZ = az / 16384.0f;
	
		AX -= AX_offset;
		AY -= AY_offset;
		AZ -= AZ_offset;
}

void MPU6050_Calibrate(void)
{
    float ax_sum = 0;
    float ay_sum = 0;
    float az_sum = 0;

    float gx_sum = 0;
    float gy_sum = 0;
    float gz_sum = 0;

    const uint16_t samples = 1000;

    for(uint16_t i = 0; i < samples; i++)
    {
        MPU6050_Read_A();
        MPU6050_Read_G();

        ax_sum += AX;
        ay_sum += AY;
        az_sum += AZ;

        gx_sum += GX;
        gy_sum += GY;
        gz_sum += GZ;

        HAL_Delay(2);
    }

    AX_offset = ax_sum / samples;
    AY_offset = ay_sum / samples;

    AZ_offset = az_sum / samples - 1.0f;

    GX_offset = gx_sum / samples;
    GY_offset = gy_sum / samples;
    GZ_offset = gz_sum / samples;
}

void MPU6050_Filter(float AX, float AY, float AZ, float GX, float GY, float GZ)
{
    float dt = 0.005f;
	
    float pitchG = pitch + GX * dt;
    float rollG  = roll  + GY * dt;

    float pitchA = atan2f(AY, sqrtf(AX * AX + AZ * AZ)) * RAD_TO_DEG;
    float rollA  = atan2f(AX, sqrtf(AY * AY + AZ * AZ)) * RAD_TO_DEG;

    pitch = (0.981f * pitchG + 0.02f * pitchA);
    roll  = (0.981f * rollG  + 0.02f * rollA);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(&huart1, &mData, 1);
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1, &mData, 1);
	MPU6050_Init();
	HAL_Delay(100);
	MPU6050_Calibrate();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		MPU6050_Read_G();
		MPU6050_Read_A();
		MPU6050_Filter(AX, AY, AZ, GX, GY, GZ);
		/*
		if(pitch - lastPitch > 1 || roll - lastRoll > 1){
			sprintf(txBuffer, "Pitch: %.2f\tRoll: %.2f\r\n", pitch, roll);
			HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, strlen(txBuffer), HAL_MAX_DELAY);
			
			lastPitch = pitch;
			lastRoll = roll;
		}
		*/
		sprintf(txBuffer, "Pitch: %.2f\tRoll: %.2f\r\n", pitch, roll);
		HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, strlen(txBuffer), HAL_MAX_DELAY);
		
		HAL_Delay(10);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
