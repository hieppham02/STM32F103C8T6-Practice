#include "ssd1315.h"

HAL_StatusTypeDef SSD1315_WriteCmd(uint8_t cmd) {
    // 0x00: Control byte cho Command
    return HAL_I2C_Mem_Write(&hi2c1, SSD1315_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

HAL_StatusTypeDef SSD1315_WriteData(uint8_t *data, uint16_t len) {
    // 0x40: Control byte cho Data
    return HAL_I2C_Mem_Write(&hi2c1, SSD1315_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

void SSD1315_Init() {
    HAL_Delay(100);
    for (uint16_t i = 0; i < sizeof(cmds); i++) {
        SSD1315_WriteCmd(cmds[i]);
    }
}

void SSD1315_Clear(void) {
    uint8_t blank[128] = {0};

    SSD1315_WriteCmd(0x21); // Set column range
    SSD1315_WriteCmd(0);
    SSD1315_WriteCmd(127);
    SSD1315_WriteCmd(0x22); // Set page range
    SSD1315_WriteCmd(0);
    SSD1315_WriteCmd(7);

    for (uint8_t page = 0; page < 8; page++) {
        SSD1315_WriteData(blank, 128);
    }
}

void SSD1315_SetPixel(uint8_t *buf, uint8_t x, uint8_t y) {
    if (x >= 128 || y >= 64) return;
    buf[x + (y / 8) * 128] |= (1 << (y % 8));
}

void SSD1315_Update(uint8_t *buf) {
    SSD1315_WriteCmd(0x21); 
		SSD1315_WriteCmd(0); 
		SSD1315_WriteCmd(127);
    SSD1315_WriteCmd(0x22); 
		SSD1315_WriteCmd(0); 
		SSD1315_WriteCmd(7);
    SSD1315_WriteData(buf, 1024);
}

void DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;

    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0); // dy ?m
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;
    int e2;

    while (1)
    {
        SSD1315_SetPixel(buf, x0, y0);

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void DrawRectangle(uint8_t *buf, int x, int y, int width, int height)
{
    DrawLine(buf, x, y, x + width, y);
    DrawLine(buf, x, y + height, x + width, y + height);
    DrawLine(buf, x, y, x, y + height);
    DrawLine(buf, x + width, y, x + width, y + height);
}

void DrawTriangle(uint8_t *buf, int x0, int y0, int x1, int y1, int x2, int y2)
{
    DrawLine(buf, x0, y0, x1, y1);
    DrawLine(buf, x1, y1, x2, y2);
    DrawLine(buf, x2, y2, x0, y0);
}

void DrawChar(uint8_t *buf, uint8_t x, uint8_t y, char c) {
    if (c < 32 || c > 127) {
        c = ' ';
    }
    uint16_t pos = (uint8_t)c - 32;

    for(uint8_t col = 0; col < 6; col ++ ){
			uint8_t colData = font68[pos][col];
			
			for(uint8_t bit = 0; bit < 8; bit ++){
					if(colData & (1 << bit)){
						SSD1315_SetPixel(buf, x + col, y + bit);
					}
			}
		}
}

void DrawString(uint8_t *buf, uint8_t x, uint8_t y, const char *str) {
    while (*str) {
      DrawChar(buf, x, y, *str);
      x += 6;
      str++;
    }
}

void ClearRegion(uint8_t *buf, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    for (uint8_t col = x; col < x + w && col < 128; col++) {
        for (uint8_t row = y; row < y + h && row < 64; row++) {
            buf[col + (row / 8) * 128] &= ~(1 << (row % 8));  // T?t pixel
        }
    }
}