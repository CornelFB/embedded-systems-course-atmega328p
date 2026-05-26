#include "terrace.h"
#include "ntc.h"
#include "fan.h"
#include "buzzer.h"
#include "roof.h"
#include "lighting.h"
#include "../drivers/lcd/lcd.h"
#include "../drivers/adc/adc.h"
#include "../drivers/timer/timer0.h"
#include "../drivers/interrupt/external_interrupt.h"
#include "../drivers/gpio/gpio.h"
 
// ADC channels
#define ADC_POT         2           // A2 - mode potentiometer (manual only)
 
// Thresholds
#define TEMP_ALARM      45
 
// Buttons
#define MODE_BUTTON_PIN     PD2     // D2 - AUTO/MANUAL toggle
#define FAN_BUTTON_PIN      PD3     // D3 - fan speed (manual only)
 
// ----------------------------------------------------------------
// Internal state
// ----------------------------------------------------------------
static terrace_mode_t   current_mode    = MODE_AUTO;
static int16_t          temperature     = 0;
 
static uint32_t         last_sensors    = 0;
static uint32_t         last_lcd        = 0;
static uint32_t         last_buzzer     = 0;
 
// ----------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------
 
/**
 * @brief Mode button callback - toggles between AUTO and MANUAL.
 *
 * Debounced to ignore presses faster than 200ms.
 */
static void mode_button_callback(void) {
    static uint32_t last_press = 0;
    uint32_t now = Millis();
 
    if (now - last_press < 200) return;
    last_press = now;
 
    current_mode = (current_mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
}
 
/**
 * @brief Fan button callback - cycles fan speed in MANUAL mode only.
 *
 * Debounced to ignore presses faster than 200ms.
 */
static void fan_button_callback(void) {
    static uint32_t last_press = 0;
    uint32_t now = Millis();
 
    if (now - last_press < 200) return;
    last_press = now;
 
    if (current_mode == MODE_MANUAL) {
        Fan_NextSpeed();
    }
}
 
/**
 * @brief Updates the LCD with the current system status.
 *
 * Row 0: temperature and mode.
 * Row 1: roof status.
 */
static void update_lcd(void) {
    LCD_SetCursor(0, 0);
    LCD_WriteString("T:");
    LCD_WriteInt(temperature);
    LCD_WriteString("C ");
    LCD_WriteString(current_mode == MODE_AUTO ? "[AUTO]  " : "[MANUAL]");
 
    LCD_SetCursor(1, 0);
    LCD_WriteString(Roof_IsClosed() ? "Roof:CLOSED " : "Roof:OPEN   ");
}
 
/**
 * @brief Reads all sensors and updates actuators accordingly.
 *
 * Runs every 500ms.
 */
static void task_sensors(void) {
    temperature = NTC_GetTemperature();
 
    Lighting_Update();
 
    if (current_mode == MODE_AUTO) {
        Roof_Update();
        Fan_Update(temperature);
    } else {
        Roof_SetManual(ADC_Read(ADC_POT));
    }
}
 
/**
 * @brief Updates buzzer state.
 *
 * Runs every 100ms.
 */
static void task_buzzer(void) {
    uint8_t alarm = (temperature >= TEMP_ALARM || Roof_IsClosed());
    Buzzer_Update(alarm);
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
    NTC_Init();
    Fan_Init();
    Buzzer_Init();
    Roof_Init();
    Lighting_Init();
 
    // Mode button on D2 with internal pull-up, triggers on falling edge
    PORTD |= (1 << MODE_BUTTON_PIN);
    ExtInt_Init(INT_0, EXT_INT_FALLING_EDGE, mode_button_callback);
 
    // Fan button on D3 with internal pull-up, triggers on falling edge
    PORTD |= (1 << FAN_BUTTON_PIN);
    ExtInt_Init(INT_1, EXT_INT_FALLING_EDGE, fan_button_callback);
 
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
        task_sensors();
    }
 
    if (now - last_lcd >= 200) {
        last_lcd = now;
        update_lcd();
    }
 
    if (now - last_buzzer >= 100) {
        last_buzzer = now;
        task_buzzer();
    }
}