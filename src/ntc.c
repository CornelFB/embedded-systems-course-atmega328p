#include "ntc.h"
#include "../drivers/adc/adc.h"
#include <math.h>
 
#define ADC_NTC         0
#define NTC_BETA        3950
#define NTC_R0          10000
#define NTC_T0          298.15f
#define NTC_RSERIES     10000
 
/**
 * @brief Initializes the ADC for NTC temperature reading.
 */
void NTC_Init(void) {
    ADC_Init();
}
 
/**
 * @brief Reads temperature from NTC thermistor on ADC channel 0.
 *
 * Uses the Beta equation to convert resistance to Celsius.
 *
 * @return int16_t Temperature in degrees Celsius.
 */
int16_t NTC_GetTemperature(void) {
    uint16_t adc = ADC_Read(ADC_NTC);
    if (adc == 0) return 0;
 
    float resistance = NTC_RSERIES * ((1023.0f / adc) - 1.0f);
    float temp_k = 1.0f / ((1.0f / NTC_T0) + (1.0f / NTC_BETA) * logf(resistance / NTC_R0));
 
    return (int16_t)(temp_k - 273.15f);
}
 