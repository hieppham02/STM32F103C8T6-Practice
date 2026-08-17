#ifndef _ST7735_H_
#define _ST7735_H_

#include <stdint.h>
#include "stm32f1xx_hal.h" 

typedef struct {
		SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* CS_Port;
    uint16_t      CS_Pin;
    GPIO_TypeDef* DC_Port;
    uint16_t      DC_Pin;
		GPIO_TypeDef* RS_Port;
    uint16_t      RS_Pin;
} ST7735_Cfg_t;

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;

extern FontDef Font_7x10; 

void LCD_Init(ST7735_Cfg_t *config);
void LCD_SendList(const uint8_t* listCMD);
void SetWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void WriteDataLCD(uint8_t data);
void WriteCommandLCD(uint8_t cmd);
void FullDisplay(uint16_t color);
void DrawChar(uint8_t x, uint8_t y, char ch, uint16_t foreground, uint16_t background);
void DrawString(uint8_t x, uint8_t y, const char* str, uint16_t foreground, uint16_t background);
void DrawImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t* img);

#endif // _ST7735_H_