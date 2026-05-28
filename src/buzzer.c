#include "buzzer.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/gpio/gpio.h"
 
#define BUZZER_PORT     GPIO_PORTD
#define BUZZER_PIN      3           // D3 - Timer2 PWM
 
static uint8_t buzzer_on = 0;
 
/**
 * @brief Initializes PWM for buzzer control.
 */
void Buzzer_Init(void) {
    PWM_Init(BUZZER_PORT, BUZZER_PIN, 2000);
    PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, 0);
}
 
/**
 * @brief Updates buzzer state.
 *
 * @param active 1 to beep, 0 to stop.
 */
void Buzzer_Update(uint8_t active) {
    if (active && !buzzer_on) {
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, 128);
        buzzer_on = 1;
    } else if (!active && buzzer_on) {
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, 0);
        buzzer_on = 0;
    }
}
 