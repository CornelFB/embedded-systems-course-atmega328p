#ifndef ROOF_H
#define ROOF_H
 
#include <stdint.h>
 
void Roof_Init(void);
void Roof_Update(void);
uint8_t Roof_IsClosed(void);
void Roof_SetManual(uint16_t pot_value);
 
#endif // ROOF_H