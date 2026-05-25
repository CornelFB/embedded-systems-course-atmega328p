
#include "terrace.h"
#include "../drivers/lcd/lcd.h"
#include "../drivers/adc/adc.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/timer/timer0.h"
#include "../drivers/gpio/gpio.h"
#include <math.h>
 
// ----------------------------------------------------------------
// Pin definitions
// ----------------------------------------------------------------
#define SERVO_PORT      GPIO_PORTB
#define SERVO_PIN       1           // D9
 
#define FAN_PORT        GPIO_PORTB
#define FAN_PIN         3           // D11
 
#define BUZZER_PORT     GPIO_PORTB
#define BUZZER_PIN      0           // D8
 
#define LED_PORT        GPIO_PORTB
#define LED_PIN         4           // D12
 
// ADC channels
#define ADC_NTC         0           // A0 - temperature sensor
#define ADC_WATER       1           // A1 - water sensor
#define ADC_POT         2           // A2 - mode potentiometer
#define ADC_LDR         3           // A3 - light sensor
 
// ----------------------------------------------------------------
// NTC configuration
// ----------------------------------------------------------------
#define NTC_BETA        3950
#define NTC_R0          10000
#define NTC_T0          298.15f
#define NTC_RSERIES     10000
 
// ----------------------------------------------------------------
// Thresholds
// ----------------------------------------------------------------
#define WATER_THRESHOLD     200
#define TEMP_FAN_START      25
#define TEMP_FAN_MAX        40
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
static uint8_t          buzzer_active   = 0;
static uint8_t          buzzer_toggle   = 0;
 
static uint32_t         last_sensors    = 0;
static uint32_t         last_lcd        = 0;
static uint32_t         last_buzzer     = 0;
 
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
 * @brief Calculates fan duty cycle based on current temperature.
 *
 * Below TEMP_FAN_START: off.
 * Between TEMP_FAN_START and TEMP_FAN_MAX: proportional 0-255.
 * Above TEMP_FAN_MAX: full speed.
 *
 * @param temp Current temperature in Celsius.
 * @return uint8_t Duty cycle (0-255).
 */
static uint8_t calculate_fan_duty(int16_t temp) {
    if (temp <= TEMP_FAN_START) return 0;
    if (temp >= TEMP_FAN_MAX)   return 255;
 
    return (uint8_t)(((int32_t)(temp - TEMP_FAN_START) * 255) /
                     (TEMP_FAN_MAX - TEMP_FAN_START));
}
 
/**
 * @brief Reads the current mode from the potentiometer.
 *
 * Below 512: AUTO mode. Above 512: MANUAL mode.
 *
 * @return terrace_mode_t Current mode.
 */
static terrace_mode_t read_mode(void) {
    return (ADC_Read(ADC_POT) < 512) ? MODE_AUTO : MODE_MANUAL;
}
 
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
    temperature  = read_temperature();
    water_val    = ADC_Read(ADC_WATER);
    is_raining   = (water_val > WATER_THRESHOLD);
    current_mode = read_mode();
 
    // LDR - turn LED on when dark
    if (ADC_Read(ADC_LDR) < LDR_THRESHOLD) {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_HIGH);
    } else {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_LOW);
    }
 
    if (current_mode == MODE_AUTO) {
        // Servo: close roof if raining
        if (is_raining && !roof_closed) {
            PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_CLOSED);
            roof_closed = 1;
        } else if (!is_raining && roof_closed) {
            PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_OPEN);
            roof_closed = 0;
        }
 
        // Fan: proportional to temperature
        PWM_SetDutyCycle(FAN_PORT, FAN_PIN, calculate_fan_duty(temperature));
 
    } else {
        // MANUAL mode: potentiometer controls servo directly
        uint16_t pot = ADC_Read(ADC_POT);
        uint8_t servo_duty = SERVO_OPEN + (uint8_t)(((uint32_t)(pot - 512) * (SERVO_CLOSED - SERVO_OPEN)) / 511);
        PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, servo_duty);
        roof_closed = (servo_duty > (SERVO_OPEN + SERVO_CLOSED) / 2);
    }
 
    // Buzzer active if raining or temperature too high
    buzzer_active = (temperature >= TEMP_ALARM || is_raining);
}
 
/**
 * @brief Buzzer task - generates an intermittent beep when active.
 *
 * Runs every 100ms.
 */
static void task_buzzer(void) {
    if (buzzer_active) {
        buzzer_toggle = !buzzer_toggle;
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, buzzer_toggle ? 128 : 0);
    } else {
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, 0);
    }
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
 
    PWM_Init(SERVO_PORT,  SERVO_PIN,  50);
    PWM_Init(FAN_PORT,    FAN_PIN,    1000);
    PWM_Init(BUZZER_PORT, BUZZER_PIN, 2000);
 
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