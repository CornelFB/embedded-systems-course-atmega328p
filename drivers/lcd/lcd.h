#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <avr/io.h>

// Pin definitions
#define LCD_RS  PD2
#define LCD_EN  PD3
#define LCD_D4  PD4
#define LCD_D5  PD5
#define LCD_D6  PD6
#define LCD_D7  PD7

// Function Prototypes
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_WriteChar(char c);
void LCD_WriteString(const char *str);
void LCD_WriteInt(int16_t value);

#endif // LCD_H