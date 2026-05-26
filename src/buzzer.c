#include "buzzer.h"
#include "../drivers/pwm/pwm.h"
#include "../drivers/gpio/gpio.h"
 
#define BUZZER_PORT     GPIO_PORTB
#define BUZZER_PIN      0           // D8
 
static uint8_t buzzer_toggle = 0;
 
/**
 * @brief Initializes PWM for buzzer control.
 */
void Buzzer_Init(void) {
    PWM_Init(BUZZER_PORT, BUZZER_PIN, 2000);
}
 
/**
 * @brief Updates buzzer state - beeps intermittently when active.
 *
 * @param active 1 to beep, 0 to stop.
 */
void Buzzer_Update(uint8_t active) {
    if (active) {
        buzzer_toggle = !buzzer_toggle;
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, buzzer_toggle ? 128 : 0);
    } else {
        PWM_SetDutyCycle(BUZZER_PORT, BUZZER_PIN, 0);
    }
}