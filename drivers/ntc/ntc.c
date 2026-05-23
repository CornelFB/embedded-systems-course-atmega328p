#include "ntc.h"
#include "drivers/adc/adc.h"
#include <math.h>

/**
 * @brief Initializes the ADC for NTC temperature reading.
 *
 * NTC is connected to pin A0 via a voltage divider with a 10kΩ series resistor.
 * VCC --- 10kΩ --- A0 --- NTC --- GND
 */
void NTC_Init(void) {
    ADC_Init();
}

/**
 * @brief Reads the NTC thermistor and returns temperature in Celsius.
 *
 * Uses the Beta equation to convert ADC reading to temperature.
 *
 * @return int16_t Temperature in degrees Celsius.
 */
int16_t NTC_GetTemperature(void) {
    uint16_t adc_value = ADC_Read(0);

    // Calculeaza rezistenta NTC din citirea ADC
    float resistance = NTC_SERIES_RESISTANCE * ((1023.0f / adc_value) - 1.0f);

    // Beta equation: 1/T = 1/T0 + (1/Beta) * ln(R/R0)
    float temp_kelvin = 1.0f / (
        (1.0f / (NTC_NOMINAL_TEMPERATURE + 273.15f)) +
        (1.0f / NTC_BETA) * log(resistance / NTC_NOMINAL_RESISTANCE)
    );

    return (int16_t)(temp_kelvin - 273.15f);
}