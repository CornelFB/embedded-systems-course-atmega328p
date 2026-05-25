#include "fan.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/gpio/gpio.h"
 
#define FAN_PORT        GPIO_PORTB
#define FAN_PIN         3           // D11
 
#define TEMP_FAN_START  25
#define TEMP_FAN_MAX    40
 
/**
 * @brief Initializes PWM for fan control.
 */
void Fan_Init(void) {
    PWM_Init(FAN_PORT, FAN_PIN, 1000);
}
 
/**
 * @brief Updates fan speed proportionally to temperature.
 *
 * Below TEMP_FAN_START: off.
 * Between TEMP_FAN_START and TEMP_FAN_MAX: proportional 0-255.
 * Above TEMP_FAN_MAX: full speed.
 *
 * @param temperature Current temperature in Celsius.
 */
void Fan_Update(int16_t temperature) {
    uint8_t duty = 0;
 
    if (temperature <= TEMP_FAN_START) {
        duty = 0;
    } else if (temperature >= TEMP_FAN_MAX) {
        duty = 255;
    } else {
        duty = (uint8_t)(((int32_t)(temperature - TEMP_FAN_START) * 255) /
                         (TEMP_FAN_MAX - TEMP_FAN_START));
    }
 
    PWM_SetDutyCycle(FAN_PORT, FAN_PIN, duty);
}