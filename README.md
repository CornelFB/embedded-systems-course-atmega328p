# Embedded Systems Course and Labs for students from Automation and Applied Informatics from Faculty of Automation, Computers and Electronics, University of Craiova

This repository is dedicated to the Embedded Systems course and labs for students from Automation and Applied Informatics from Faculty of Automation, Computers and Electronics, University of Craiova. 

If you are a student: please fork this repository and use it for your labs, homework and course. 

Found a bug or you just want to contribute to this project ? Please raise an issue and/or send a pull request.

[![Run Tests](https://github.com/mamuleanu/embedded-systems-course-atmega328p/actions/workflows/tests.yml/badge.svg)](https://github.com/mamuleanu/embedded-systems-course-atmega328p/actions/workflows/tests.yml)

## About This Fork
This fork contains the implementation of the Smart Restaurant Terrace project — an automated terrace management system for a restaurant, built on Arduino Uno without any Arduino libraries.

The system monitors temperature and rain conditions to automatically control a roof cover (servo motor) and a cooling fan. It also features automatic lighting based on ambient light, buzzer alerts, and a manual override mode for direct control.

## Features

- **No Arduino Libraries**: Direct register manipulation for maximum control and efficiency.
- **Drivers:**: Modular, documented, and reusable.
    - **GPIO**: Initialization, Write, Read, Toggle.
    - **Interrupts**: External Interrupts (INT0, INT1) with callback support.
    - **Timer**: 1ms System Tick (`Millis()`) using Timer0 CTC mode.
    - **EEPROM**: Read, Write, Update (lifespan-aware).
    - **ADC**: Blocking 10-bit Analog-to-Digital conversion.
    - **PWM**: High-level wrapper for Timer1 (16-bit) and Timer2 (8-bit) PWM generation.
    - **LCD**: HD44780 16x2 LCD driver over I2C (PCF8574 module).
- **Board Support Package (BSP)**: Pin mappings for **Arduino Nano** and **Uno**.
- **Robust Build System**: `Makefile` for compilation, flashing, and testing.
- **Host-Based Unit Testing**: Run unit tests on your computer without hardware using register mocking.
- **Code Coverage**: Generate HTML reports (`lcov`) to verify test coverage.

## Roadmap

- [x] GPIO driver
- [x] ADC driver
- [x] EEPROM driver
- [x] Interrupt driver
- [x] Timer driver
- [x] PWM driver
- [X] LCD driver (I2C)
- [ ] SPI driver
- [ ] UART driver
- [ ] Unit tests

## Project: Smart Restaurant Terrace

What it does

-Temperature monitoring via NTC thermistor — fan starts automatically when too hot
-Rain detection via water sensor — servo motor closes the roof automatically
-Automatic lighting via LDR — LED turns on when dark
-Buzzer alerts for rain and high temperature events
-AUTO / MANUAL mode switchable via a button (INT0)
-Manual fan speed control via a second button (INT1) — cycles OFF → MEDIUM → MAX
-Manual roof control via potentiometer in MANUAL mode
-LCD 16x2 (I2C) displays temperature, mode and roof status in real time
## Hardware
Component                 Pin
NTC Thermistor            A0
Water Sensor              A1
Potentiometer             A2
LDR                       A3
LCD SDA                   A4
LCD SCL                   A5
Mode Button (AUTO/MANUAL) D2
Fan Speed Button (MANUAL) D3
Buzzer                    D8
Servo Motor               D9
Fan DC 5V                 D11
LED                       D12

## Project Structure

```
├── bsp/            # Board definitions (uno.h, nano.h)
├── drivers/        # Hardware Abstraction Layer
│   ├── adc/
│   ├── eeprom/
│   ├── gpio/
│   ├── interrupt/
│   ├── lcd/
│   └── timer/
├── src/            # Application source code
│   ├── main.c
│   ├── terrace.c/.h    # Main scheduler and mode logic
│   ├── ntc.c/.h        # Temperature reading
│   ├── fan.c/.h        # Fan speed control
│   ├── buzzer.c/.h     # Buzzer alerts
│   ├── roof.c/.h       # Servo + water sensor
│   └── lighting.c/.h   # LDR + LED
├── test/           # Unit tests & Mocks
│   ├── mocks/      # Mock AVR registers for host testing
│   ├── framework/  # Minimal test runner
│   └── test_*.c    # Unit test files
├── utils/          # Helper macros (BIT manipulations)
└── Makefile        # Build configuration
```

## Build & Flash

### Prerequisites
- `avr-gcc` toolchain
- `avrdude`
- `make`

### Commands
| Command | Description |
|---------|-------------|
| `make all BOARD=uno` | Compile the project for Arduino Nano. |
| `make flash` | Flash the firmware to the connected board. |
| `make clean` | Remove build artifacts. |

## Testing & Coverage

This project supports running unit tests on your host machine (Mac/Linux) by mocking the AVR hardware registers.

### Prerequisites (for coverage)
- `gcc`
- `lcov` (`brew install lcov`)

### Commands
| Command | Description |
|---------|-------------|
| `make test` | Compile and run all unit tests (GPIO, PWM) on the host. |
| `make coverage` | Run tests and generate usage metrics. |
| `make coverage-html` | Generate a visual HTML report of code coverage. |

![Code Coverage Example](/img/code_coverage_example.png)

## Usage Example

```c
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"

int main(void) {
    
    Timer0_Init();
    GPIO_Init(LED_BUILTIN, GPIO_OUTPUT);

    uint32_t last_time = 0;

    while (1) {
            
        if (Millis() - last_time >= 1000) {
            last_time = Millis();
            GPIO_Toggle(LED_BUILTIN);
        }
    }
}

// PWM Usage Example
#include "drivers/pwm/pwm.h"
#include "bsp/uno.h"

int pwm_example(void) {
    // 50Hz for Servo on D9
    PWM_Init(UNO_D9, 50);
    // 1.5ms pulse (approx neutral)
    // Duty cycle calculation: (1.5ms / 20ms) * ICR1_TOP
    // Wrapper takes 0-255: (1.5/20)*255 = ~19
    PWM_SetDutyCycle(UNO_D9, 19);

    // 1kHz LED Dimming on D11
    PWM_Init(UNO_D11, 1000);
    PWM_SetDutyCycle(UNO_D11, 128); // 50%
    
    return 0;
}
```

