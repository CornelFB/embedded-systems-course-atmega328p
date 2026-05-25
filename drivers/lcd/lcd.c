#include "lcd.h"
#include <util/delay.h>
#include <avr/io.h>
 
// PCF8574 pin mapping to HD44780
// Bit 0 (P0) = RS
// Bit 2 (P2) = EN
// Bit 3 (P3) = Backlight
// Bit 4-7    = D4-D7
 
#define LCD_BACKLIGHT   0x08
#define LCD_EN          0x04
#define LCD_RS          0x01
 
// ----------------------------------------------------------------
// I2C low-level functions (built-in, no separate driver needed)
// ----------------------------------------------------------------
 
/**
 * @brief Initializes the I2C peripheral at 100kHz.
 */
static void i2c_init(void) {
    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}
 
/**
 * @brief Sends a START condition on the I2C bus.
 */
static void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}
 
/**
 * @brief Sends a STOP condition on the I2C bus.
 */
static void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}
 
/**
 * @brief Sends a byte over I2C.
 *
 * @param data Byte to send.
 */
static void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}
 
// ----------------------------------------------------------------
// PCF8574 + LCD low-level functions
// ----------------------------------------------------------------
 
/**
 * @brief Sends a byte to the PCF8574 over I2C.
 *
 * @param data Byte to send.
 */
static void pcf8574_write(uint8_t data) {
    i2c_start();
    i2c_write((LCD_I2C_ADDR << 1) | 0);
    i2c_write(data | LCD_BACKLIGHT);
    i2c_stop();
}
 
/**
 * @brief Pulses the EN pin to latch data into the LCD.
 *
 * @param data Current data byte on the bus.
 */
static void lcd_pulse_enable(uint8_t data) {
    pcf8574_write(data | LCD_EN);
    _delay_us(1);
    pcf8574_write(data & ~LCD_EN);
    _delay_us(50);
}
 
/**
 * @brief Sends a 4-bit nibble to the LCD.
 *
 * @param nibble Nibble to send (already in upper 4 bits position).
 */
static void lcd_send_nibble(uint8_t nibble) {
    pcf8574_write(nibble);
    lcd_pulse_enable(nibble);
}
 
/**
 * @brief Sends a full byte to the LCD in two nibbles.
 *
 * @param byte    Byte to send.
 * @param is_data 1 for data register, 0 for instruction register.
 */
static void lcd_send_byte(uint8_t byte, uint8_t is_data) {
    uint8_t rs = is_data ? LCD_RS : 0;
 
    lcd_send_nibble((byte & 0xF0) | rs);
    lcd_send_nibble(((byte << 4) & 0xF0) | rs);
}
 
// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------
 
/**
 * @brief Initializes the LCD in 4-bit mode over I2C.
 *
 * Must be called once before any other LCD function.
 */
void LCD_Init(void) {
    i2c_init();
    _delay_ms(50);
 
    lcd_send_nibble(0x30); _delay_ms(5);
    lcd_send_nibble(0x30); _delay_ms(1);
    lcd_send_nibble(0x30); _delay_ms(1);
    lcd_send_nibble(0x20); _delay_ms(1);
 
    lcd_send_byte(0x28, 0); _delay_ms(1);
    lcd_send_byte(0x08, 0); _delay_ms(1);
    lcd_send_byte(0x01, 0); _delay_ms(2);
    lcd_send_byte(0x06, 0); _delay_ms(1);
    lcd_send_byte(0x0C, 0); _delay_ms(1);
}
 
/**
 * @brief Clears the display and returns cursor to home.
 */
void LCD_Clear(void) {
    lcd_send_byte(0x01, 0);
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
    lcd_send_byte(0x80 | addr, 0);
    _delay_us(50);
}
 
/**
 * @brief Writes a single character at the current cursor position.
 *
 * @param c The character to write.
 */
void LCD_WriteChar(char c) {
    lcd_send_byte((uint8_t)c, 1);
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