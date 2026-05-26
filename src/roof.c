#include "roof.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/adc/adc.h"
#include "../drivers/gpio/gpio.h"
 
#define SERVO_PORT      GPIO_PORTB
#define SERVO_PIN       1           // D9
 
#define ADC_WATER       1           // A1 - water sensor
#define WATER_THRESHOLD 500
 
#define SERVO_OPEN      13
#define SERVO_CLOSED    26
 
static uint8_t roof_closed = 0;
 
/**
 * @brief Initializes PWM for servo control and opens the roof.
 */
void Roof_Init(void) {
    PWM_Init(SERVO_PORT, SERVO_PIN, 50);
    PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_OPEN);
    roof_closed = 0;
}
 
/**
 * @brief Updates roof position automatically based on water sensor.
 *
 * Closes roof if rain detected, opens if dry.
 */
void Roof_Update(void) {
    uint8_t is_raining = (ADC_Read(ADC_WATER) > WATER_THRESHOLD);
 
    if (is_raining && !roof_closed) {
        PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_CLOSED);
        roof_closed = 1;
    } else if (!is_raining && roof_closed) {
        PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, SERVO_OPEN);
        roof_closed = 0;
    }
}
 
/**
 * @brief Controls roof position manually via potentiometer.
 *
 * @param pot_value ADC value from potentiometer (0-1023).
 */
void Roof_SetManual(uint16_t pot_value) {
    uint8_t servo_duty = SERVO_OPEN + (uint8_t)(((uint32_t)pot_value * (SERVO_CLOSED - SERVO_OPEN)) / 1023);
    PWM_SetDutyCycle(SERVO_PORT, SERVO_PIN, servo_duty);
    roof_closed = (servo_duty > (SERVO_OPEN + SERVO_CLOSED) / 2);
}
 
/**
 * @brief Returns current roof state.
 *
 * @return uint8_t 1 if closed, 0 if open.
 */
uint8_t Roof_IsClosed(void) {
    return roof_closed;
}