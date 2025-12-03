// STM32L432KC_GPIO.h
// Header for GPIO functions

#ifndef STM32L4_GPIO_H
#define STM32L4_GPIO_H

#include <stdint.h> // Include stdint header

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

///////////////////////////////////////////////////////////////////////////////
// Type definitions - only if CMSIS headers not already included
///////////////////////////////////////////////////////////////////////////////

#ifndef __STM32L432xx_H
// Only define these if standard CMSIS library (stm32l432xx.h) is NOT included
// This prevents redefinition errors when using USART library

// Base addresses for GPIO ports
#define GPIOA_BASE  (0x48000000UL)
#define GPIOB_BASE  (0x48000400UL)

// GPIO register structs here
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PURPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_TypeDef;

// Pointers to GPIO-sized chunks of memory for each peripheral
#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIO GPIOA

#endif // __STM32L432xx_H

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void pinMode(int pin, int function);

int digitalRead(int pin);

void digitalWrite(int pin, int val);

void togglePin(int pin);

void initAltFxn(void);



#endif