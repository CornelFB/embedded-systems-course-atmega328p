#include "lighting.h"
#include "../drivers/adc/adc.h"
#include "../drivers/gpio/gpio.h"
 
#define LED_PORT        GPIO_PORTB
#define LED_PIN         4           // D12
 
#define ADC_LDR         3           // A3 - light sensor
#define LDR_THRESHOLD   400
 
/**
 * @brief Initializes the LED pin as output.
 */
void Lighting_Init(void) {
    GPIO_Init(LED_PORT, LED_PIN, GPIO_OUTPUT);
    GPIO_Write(LED_PORT, LED_PIN, GPIO_LOW);
}
 
/**
 * @brief Turns LED on when dark, off when bright.
 */
void Lighting_Update(void) {
    if (ADC_Read(ADC_LDR) < LDR_THRESHOLD) {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_HIGH);
    } else {
        GPIO_Write(LED_PORT, LED_PIN, GPIO_LOW);
    }
}