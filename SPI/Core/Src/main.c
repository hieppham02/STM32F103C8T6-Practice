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
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Init();
void SetWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void WriteDataLCD(uint8_t data);
void WriteCommandLCD(uint8_t cmd);

void LCD_Init(void)
{
	// Hardware Reset
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_Delay(20);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_Delay(150);

	// Software Reset
	WriteCommandLCD(0x01);
	HAL_Delay(150);

	// Sleep Out
	WriteCommandLCD(0x11);
	HAL_Delay(150);

	// Frame Rate Control - Normal mode
	WriteCommandLCD(0xB1);
	WriteDataLCD(0x01);
	WriteDataLCD(0x2C);
	WriteDataLCD(0x2D);

	// Frame Rate Control - Idle mode
	WriteCommandLCD(0xB2);
	WriteDataLCD(0x01);
	WriteDataLCD(0x2C);
	WriteDataLCD(0x2D);

	// Frame Rate Control - Partial mode
	WriteCommandLCD(0xB3);
	WriteDataLCD(0x01);
	WriteDataLCD(0x2C);
	WriteDataLCD(0x2D);
	WriteDataLCD(0x01);
	WriteDataLCD(0x2C);
	WriteDataLCD(0x2D);

	// Display inversion control
	WriteCommandLCD(0xB4);
	WriteDataLCD(0x07);

	// Power control 1
	WriteCommandLCD(0xC0);
	WriteDataLCD(0xA2);
	WriteDataLCD(0x02);
	WriteDataLCD(0x84);

	// Power control 2
	WriteCommandLCD(0xC1);
	WriteDataLCD(0xC5);

	// Power control 3
	WriteCommandLCD(0xC2);
	WriteDataLCD(0x0A);
	WriteDataLCD(0x00);

	// Power control 4
	WriteCommandLCD(0xC3);
	WriteDataLCD(0x8A);
	WriteDataLCD(0x2A);

	// Power control 5
	WriteCommandLCD(0xC4);
	WriteDataLCD(0x8A);
	WriteDataLCD(0xEE);

	// VCOM control
	WriteCommandLCD(0xC5);
	WriteDataLCD(0x0E);

	// Gamma +
	WriteCommandLCD(0xE0);
	WriteDataLCD(0x02);
	WriteDataLCD(0x1C);
	WriteDataLCD(0x07);
	WriteDataLCD(0x12);
	WriteDataLCD(0x37);
	WriteDataLCD(0x32);
	WriteDataLCD(0x29);
	WriteDataLCD(0x2D);
	WriteDataLCD(0x29);
	WriteDataLCD(0x25);
	WriteDataLCD(0x2B);
	WriteDataLCD(0x39);
	WriteDataLCD(0x00);
	WriteDataLCD(0x01);
	WriteDataLCD(0x03);
	WriteDataLCD(0x10);

	// Gamma -
	WriteCommandLCD(0xE1);
	WriteDataLCD(0x03);
	WriteDataLCD(0x1D);
	WriteDataLCD(0x07);
	WriteDataLCD(0x06);
	WriteDataLCD(0x2E);
	WriteDataLCD(0x2C);
	WriteDataLCD(0x29);
	WriteDataLCD(0x2D);
	WriteDataLCD(0x2E);
	WriteDataLCD(0x2E);
	WriteDataLCD(0x37);
	WriteDataLCD(0x3F);
	WriteDataLCD(0x00);
	WriteDataLCD(0x00);
	WriteDataLCD(0x02);
	WriteDataLCD(0x10);

	// Color mode: RGB565
	WriteCommandLCD(0x3A);
	WriteDataLCD(0x05);

	// Memory Access Control
	WriteCommandLCD(0x36);
	WriteDataLCD(0xC0);

	// Normal display mode
	WriteCommandLCD(0x13);
	HAL_Delay(10);

	// Inversion off
	WriteCommandLCD(0x20);
	HAL_Delay(100);

	SetWindow(0, 0, 128 - 1, 160 - 1);

	// Display ON
	WriteCommandLCD(0x29);
	HAL_Delay(100);
}

void SetWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
	WriteCommandLCD(0x2A);      // Set Collumn
	WriteDataLCD(0x00);         // Data
	WriteDataLCD(x1);           // Data
	WriteDataLCD(0x00);         // Data
	WriteDataLCD(x2);           // Data
	
	WriteCommandLCD(0x2B);      // Set Row
	WriteDataLCD(0x00);         // Data
	WriteDataLCD(y1);           // Data
	WriteDataLCD(0x00);         // Data
	WriteDataLCD(y2);           // Data
}

void WriteDataLCD(uint8_t data){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 1);
	HAL_SPI_Transmit(&hspi1, &data, 1, 1000); 
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
}

void WriteCommandLCD(uint8_t cmd){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 0);
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
}

void FullDisplay(uint16_t color){
	// Buffer: 128 pixel * 2 byte/pixel = 256 bytes
	uint8_t line_buffer[256];
	uint8_t color_high = color >> 8;
	uint8_t color_low = color & 0xFF;
	// Ve 1 hang 128 pixels
	for (uint16_t i = 0; i < 128; i++) {
		line_buffer[i * 2] = color_high;
		line_buffer[i * 2 + 1] = color_low;
	}
	WriteCommandLCD(0x2C);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // CS Low
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);   // DC high => data mode
	// ve 160 hang doc
	// ******..****** = 128
	// ..............
	// ******..******
	//  ||
	// 160
	for (uint16_t y = 0; y < 160; y++) {
		HAL_SPI_Transmit_DMA(&hspi1, line_buffer, 256);
		while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // CS High
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
  MX_DMA_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
	LCD_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		FullDisplay(0x04d9);
		HAL_Delay(100);
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

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
