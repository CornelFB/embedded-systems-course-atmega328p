#ifndef BUZZER_H
#define BUZZER_H
 
#include <stdint.h>
 
void Buzzer_Init(void);
void Buzzer_Update(uint8_t active);
 
#endif // BUZZER_H