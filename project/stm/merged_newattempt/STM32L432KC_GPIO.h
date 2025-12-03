// STM32L432KC_GPIO.h
// Header for GPIO functions

#ifndef STM32L4_GPIO_H
#define STM32L4_GPIO_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS device header (provides GPIO_TypeDef, GPIOA, etc.)

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// Values for GPIO pins ("val" arguments)
#define GPIO_LOW    0
#define GPIO_HIGH   1

// Arbitrary GPIO functions for pinMode()
#define GPIO_INPUT  0
#define GPIO_OUTPUT 1
#define GPIO_ALT    2
#define GPIO_ANALOG 3

// Pin definitions
#define PA0  0
#define PA1  1
#define PA2  2
#define PA3  3
#define PA4  4
#define PA5  5
#define PA6  6
#define PA7  7
#define PA8  8
#define PA9  9
#define PA10 10
#define PA11 11
#define PA12 12
#define PA13 13
#define PA14 14
#define PA15 15

#define PB0  0
#define PB1  1
#define PB3  3
#define PB4  4
#define PB5  5
#define PB6  6
#define PB7  7

// GPIO port selection
#define GPIO_PORT_A 0
#define GPIO_PORT_B 1

// Default GPIO port for backward compatibility
#define GPIO GPIOA

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void gpioEnable(int port);

void pinMode(int pin, int function);

int digitalRead(int pin);

void digitalWrite(int pin, int val);

void togglePin(int pin);

void initAltFxn(void);

#endif