#ifndef FAN_H
#define FAN_H
 
#include <stdint.h>
 
typedef enum {
    FAN_OFF,
    FAN_MEDIUM,
    FAN_MAX
} fan_speed_t;
 
void Fan_Init(void);
void Fan_Update(int16_t temperature);
void Fan_NextSpeed(void);
fan_speed_t Fan_GetSpeed(void);
 
#endif // FAN_H