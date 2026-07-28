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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1315.h"
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

#define SRF05_TRIG_PORT GPIOA
#define SRF05_TRIG_PIN  GPIO_PIN_5

#define SRF05_ECHO_PORT GPIOA
#define SRF05_ECHO_PIN  GPIO_PIN_6

uint8_t oled_buf[1024];

volatile uint8_t flag = 0;

float distance = 0.0f;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        flag = 1;
    }
}

void Draw1(char* str){
    ClearRegion(oled_buf, 0, 10, 128, 48);
    uint8_t text_width = (uint8_t)strlen(str) * 6;
    uint8_t x = (128 - text_width) / 2;
    
    DrawRectangle(oled_buf, x - 5, 25, text_width + 10, 15);
    DrawString(oled_buf, x, 30, str);
}

void Delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}

void SRF05_Trigger(void)
{
    HAL_GPIO_WritePin(SRF05_TRIG_PORT, SRF05_TRIG_PIN, 0);
    Delay_us(2);

    HAL_GPIO_WritePin(SRF05_TRIG_PORT, SRF05_TRIG_PIN, 1);
    Delay_us(10);

    HAL_GPIO_WritePin(SRF05_TRIG_PORT, SRF05_TRIG_PIN, 0);
}
uint32_t elapsed;
float GetDistance() {
    elapsed  = 0;
    uint32_t timeout = 30000; // 30ms

    SRF05_Trigger();

    // ECHO -> HIGH
		// wait gpio high -> bat dau do
    while(HAL_GPIO_ReadPin(SRF05_ECHO_PORT, SRF05_ECHO_PIN) == 0) {
        if(--timeout == 0) return 0;
    }

		// reset counter
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    // 3. ECHO -> LOW
		// wait gpio low -> ket thuc do
    while (HAL_GPIO_ReadPin(SRF05_ECHO_PORT, SRF05_ECHO_PIN) == 1) {
        if (__HAL_TIM_GET_COUNTER(&htim3) > 30000) return 0; // Qu� xa (> 5 m�t)
    }

    // ECHO LOW => lay thoi gian tu counter
     elapsed  = __HAL_TIM_GET_COUNTER(&htim3);

		// V = 34.3 m/s = 34300 cm/s 
		// 1s = 1,000,000 microsecond
		// => V = 0.034 cm/microsecond
		// chia 2 vi tinh 2 quang duong di + ve
    return (float)elapsed  * (34300.0f / 1000000.0f) / 2 ;
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2); 
	HAL_TIM_Base_Start(&htim3);
	
	SSD1315_Init();
  SSD1315_Clear();

  memset(oled_buf, 0x00, sizeof(oled_buf));
	//DrawTriangle(oled_buf, 35, 12, 35, 56, 93, 39);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		distance = GetDistance();
    if(distance <= 5.0f){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);
		} else {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0);
		}
    char text[20];
    sprintf(text, "%.1f cm\n", distance);
    Draw1(text);
    if (flag) { 
      flag = 0;
      SSD1315_Update(oled_buf); 
			HAL_UART_Transmit(&huart1, (uint8_t*)text, strlen(text), 100);
    }
    
    HAL_Delay(60);
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
