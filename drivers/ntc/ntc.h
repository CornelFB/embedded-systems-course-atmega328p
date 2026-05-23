#ifndef NTC_H
#define NTC_H

#include <stdint.h>
#include <avr/io.h>

// NTC termistor parameters
#define NTC_NOMINAL_RESISTANCE  10000   // Rezistenta nominala la 25°C (10kΩ)
#define NTC_NOMINAL_TEMPERATURE 25      // Temperatura nominala (°C)
#define NTC_BETA                3950    // Coeficientul Beta al termistorului
#define NTC_SERIES_RESISTANCE   10000   // Rezistenta in serie (10kΩ)

// Function Prototypes
void NTC_Init(void);
int16_t NTC_GetTemperature(void);

#endif // NTC_H