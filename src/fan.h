#ifndef FAN_H
#define FAN_H
 
#include <stdint.h>
 
void Fan_Init(void);
void Fan_Update(int16_t temperature);
 
#endif // FAN_H
 