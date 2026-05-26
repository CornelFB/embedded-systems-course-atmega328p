#include "terrace.h"
#include "ntc.h"
#include "fan.h"
#include "buzzer.h"
#include "../drivers/lcd/lcd.h"
#include "../drivers/adc/adc.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/timer/timer0.h"
#include "../drivers/gpio/gpio.h"
 
// ----------------------------------------------------------------
// Pin definitions
// ----------------------------------------------------------------
#define SERVO_PORT      GPIO_PORTB
#define SERVO_PIN       1           // D9
 
#define LED_PORT        GPIO_PORTB
#define LED_PIN         4           // D12
 
// ADC channels
#define ADC_WATER       1           // A1 - water sensor
#define ADC_POT         2           // A2 - mode potentiometer
#define ADC_LDR         3           // A3 - light sensor
 
// ----------------------------------------------------------------
// Thresholds
// ----------------------------------------------------------------
#define WATER_THRESHOLD     500
#define TEMP_ALARM          45
#define LDR_THRESHOLD       400
 
// Servo duty cycle
#define SERVO_OPEN          13
#define SERVO_CLOSED        26
 
// ----------------------------------------------------------------
// Internal state
// ----------------------------------------------------------------
static terrace_mode_t   current_mode    = MODE_AUTO;
static int16_t          temperature     = 0;
static uint16_t         water_val       = 0;
static uint8_t          is_raining      = 0;
static uint8_t          roof_closed     = 0;
 
static uint32_t         last_sensors    = 0;
static uint32_t         last_lcd        = 0;
static uint32_t         last_buzzer     = 0;
 
// ----------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------
 
/**
 * @brief Updates the LCD with the current system status.
 *
 * Row 0: temperature and mode.
 * Row 1: roof status and rain indicator.
 */
static void update_lcd(void) {
    LCD_SetCursor(0, 0);
    LCD_WriteString("T:");
    LCD_WriteInt(temperature);
    LCD_WriteString("C ");
    LCD_WriteString(current_mode == MODE_AUTO ? "[AUTO]  " : "[MANUAL]");
 
    LCD_SetCursor(1, 0);
    LCD_WriteString(roof_closed ? "Roof:CLOSED " : "Roof:OPEN   ");
    LCD_WriteString(is_raining  ? "RAIN" : "    ");
}
 
/**
 * @brief Reads all sensors and updates actuators accordingly.
 *
 * Runs every 500ms.
 */
static void task_sensors(void) {
    temperature  = NTC_GetTemperature();
    water_val    = ADC_Read(ADC_WATER);
    is_raining   = (water_val > WATER_THRESHOLD);
    current_mode = (ADC_Read(ADC_POT) < 512) ? MODE_AUTO : MODE_MANUAL;
 
    // LDR - turn LED on when dark
    if (ADC_Read(ADC_LDR) < LDR_THRESHOLD) {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_HIGH);
    } else {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_LOW);
    }
 
    if (current_mode == MODE_AUTO) {
        if (is_raining && !roof_closed) {
            PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_CLOSED);
            roof_closed = 1;
        } else if (!is_raining && roof_closed) {
            PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_OPEN);
            roof_closed = 0;
        }
 
        Fan_Update(temperature);
 
    } else {
        uint16_t pot = ADC_Read(ADC_POT);
        uint8_t servo_duty = SERVO_OPEN + (uint8_t)(((uint32_t)(pot - 512) * (SERVO_CLOSED - SERVO_OPEN)) / 511);
        PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, servo_duty);
        roof_closed = (servo_duty > (SERVO_OPEN + SERVO_CLOSED) / 2);
    }
}
 
/**
 * @brief Updates buzzer state.
 *
 * Runs every 100ms.
 */
static void task_buzzer(void) {
    uint8_t alarm = (temperature >= TEMP_ALARM || is_raining);
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
 
    PWM_Init(SERVO_PORT, SERVO_PIN, 50);
 
    GPIO_Init(LED_PORT, LED_PIN, GPIO_OUTPUT);
    GPIO_Write(LED_PORT, LED_PIN, GPIO_LOW);
 
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Smart Restaurant");
    LCD_SetCursor(1, 0);
    LCD_WriteString("    Terrace     ");
 
    PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_OPEN);
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