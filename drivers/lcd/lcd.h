#ifndef LCD_H
#define LCD_H
 
#include <stdint.h>
#include <avr/io.h>
 
// I2C address of PCF8574 module (default 0x27, try 0x3F if not working)
#define LCD_I2C_ADDR    0x27
 
// Function Prototypes
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_WriteChar(char c);
void LCD_WriteString(const char *str);
void LCD_WriteInt(int16_t value);
 
#endif // LCD_H