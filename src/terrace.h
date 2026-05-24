#ifndef TERRACE_H
#define TERRACE_H
 
#include <stdint.h>
 //mod de functionarea terasei
typedef enum {
    MODE_AUTO,
    MODE_MANUAL
} terrace_mode_t;
 
void Terrace_Init(void);
void Terrace_Run(void);
 
#endif // TERRACE_H
 