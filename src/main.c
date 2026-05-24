#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"
/*

int main(void) {
    
    Timer0_Init();

    
    GPIO_Init(LED_BUILTIN, GPIO_OUTPUT);

//The Smart Restaurant Terrace

    uint32_t last_time = 0;

    while (1) {
            
        if (Millis() - last_time >= 1000) {
            last_time = Millis();
            GPIO_Toggle(LED_BUILTIN);
        }
    }
}
*/
#include "terrace.h"

int main(void) {
    Terrace_Init();

    while (1) {
        Terrace_Run();
    }
    return 0;
}