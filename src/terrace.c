#include "terrace.h"
#include "../drivers/lcd/lcd.h"
#include "../drivers/adc/adc.h"
#include "../drivers/timer/timer0.h"
 
// ----------------------------------------------------------------
// ADC channels
// ----------------------------------------------------------------
#define ADC_NTC         0           // A0 - temperature sensor
 
// ----------------------------------------------------------------
// NTC configuration
// ----------------------------------------------------------------
#define NTC_BETA        3950
#define NTC_R0          10000
#define NTC_T0          298.15f
#define NTC_RSERIES     10000
 
// ----------------------------------------------------------------
// Internal state
// ----------------------------------------------------------------
static int16_t temperature = 0;
 
static uint32_t last_sensors = 0;
static uint32_t last_lcd     = 0;
 
// ----------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------
 
/**
 * @brief Reads temperature from NTC thermistor on ADC channel 0.
 *
 * Uses the Beta equation to convert resistance to Celsius.
 *
 * @return int16_t Temperature in degrees Celsius.
 */
static int16_t read_temperature(void) {
    uint16_t adc = ADC_Read(ADC_NTC);
    if (adc == 0) return 0;
 
    float resistance = NTC_RSERIES * ((1023.0f / adc) - 1.0f);
    float temp_k = 1.0f / ((1.0f / NTC_T0) + (1.0f / NTC_BETA) * logf(resistance / NTC_R0));
 
    return (int16_t)(temp_k - 273.15f);
}
 
/**
 * @brief Updates the LCD with the current temperature.
 */
static void update_lcd(void) {
    LCD_SetCursor(0, 0);
    LCD_WriteString("Temp: ");
    LCD_WriteInt(temperature);
    LCD_WriteString("C      ");
 
    LCD_SetCursor(1, 0);
    LCD_WriteString("Initializing... ");
}
 
// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------
 
/**
 * @brief Initializes all system modules.
 */
void Terrace_Init(void) {
    ADC_Init();
    Timer0_Init();
 
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Smart Restaurant");
    LCD_SetCursor(1, 0);
    LCD_WriteString("    Terrace     ");
}
 
/**
 * @brief Main application loop - Millis() based scheduler.
 */
void Terrace_Run(void) {
    uint32_t now = Millis();
 
    if (now - last_sensors >= 500) {
        last_sensors = now;
        temperature = read_temperature();
    }
 
    if (now - last_lcd >= 200) {
        last_lcd = now;
        update_lcd();
    }
}
 