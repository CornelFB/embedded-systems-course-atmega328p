#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

/**
 * @brief Pulses the EN pin to latch data into the LCD.
 */
static void LCD_PulseEnable(void) {
    PORTD |=  (1 << LCD_EN);
    _delay_us(1);
    PORTD &= ~(1 << LCD_EN);
    _delay_us(50);
}

/**
 * @brief Sends a 4-bit nibble to the LCD data pins.
 *
 * @param nibble The lower 4 bits to send.
 */
static void LCD_SendNibble(uint8_t nibble) {
    PORTD &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));

    if (nibble & 0x01) PORTD |= (1 << LCD_D4);
    if (nibble & 0x02) PORTD |= (1 << LCD_D5);
    if (nibble & 0x04) PORTD |= (1 << LCD_D6);
    if (nibble & 0x08) PORTD |= (1 << LCD_D7);

    LCD_PulseEnable();
}

/**
 * @brief Sends a full byte to the LCD in two nibbles.
 *
 * @param byte    The byte to send.
 * @param is_data 1 for data register, 0 for instruction register.
 */
static void LCD_SendByte(uint8_t byte, uint8_t is_data) {
    if (is_data) {
        PORTD |=  (1 << LCD_RS);
    } else {
        PORTD &= ~(1 << LCD_RS);
    }

    LCD_SendNibble(byte >> 4);
    LCD_SendNibble(byte & 0x0F);
}

/**
 * @brief Initializes the LCD in 4-bit mode.
 *
 * Must be called once before any other LCD function.
 */
void LCD_Init(void) {
    DDRD |= (1 << LCD_RS) | (1 << LCD_EN) |
            (1 << LCD_D4) | (1 << LCD_D5) |
            (1 << LCD_D6) | (1 << LCD_D7);

    PORTD &= ~((1 << LCD_RS) | (1 << LCD_EN) |
               (1 << LCD_D4) | (1 << LCD_D5) |
               (1 << LCD_D6) | (1 << LCD_D7));

    _delay_ms(50);

    LCD_SendNibble(0x03); _delay_ms(5);
    LCD_SendNibble(0x03); _delay_ms(1);
    LCD_SendNibble(0x03); _delay_ms(1);
    LCD_SendNibble(0x02); _delay_ms(1);

    LCD_SendByte(0x28, 0); _delay_ms(1);
    LCD_SendByte(0x08, 0); _delay_ms(1);
    LCD_SendByte(0x01, 0); _delay_ms(2);
    LCD_SendByte(0x06, 0); _delay_ms(1);
    LCD_SendByte(0x0C, 0); _delay_ms(1);
}

/**
 * @brief Clears the display and returns cursor to home.
 */
void LCD_Clear(void) {
    LCD_SendByte(0x01, 0);
    _delay_ms(2);
}

/**
 * @brief Sets the cursor to a specific position.
 *
 * @param row 0 for first row, 1 for second row.
 * @param col Column position (0-15).
 */
void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? col : (0x40 + col);
    LCD_SendByte(0x80 | addr, 0);
    _delay_us(50);
}

/**
 * @brief Writes a single character at the current cursor position.
 *
 * @param c The character to write.
 */
void LCD_WriteChar(char c) {
    LCD_SendByte((uint8_t)c, 1);
    _delay_us(50);
}

/**
 * @brief Writes a null-terminated string at the current cursor position.
 *
 * @param str The string to write.
 */
void LCD_WriteString(const char *str) {
    while (*str) {
        LCD_WriteChar(*str++);
    }
}

/**
 * @brief Writes a signed integer at the current cursor position.
 *
 * @param value The integer to write.
 */
void LCD_WriteInt(int16_t value) {
    char buf[7];
    int8_t i = 0;

    if (value < 0) {
        LCD_WriteChar('-');
        value = -value;
    }

    if (value == 0) {
        LCD_WriteChar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        LCD_WriteChar(buf[--i]);
    }
}