// STM32L432KC_GPIO.c
// Source code for GPIO functions

#include "STM32L432KC_GPIO.h"
#include <stdio.h>


void pinMode(int pin, int function) {
    switch(function) {
        case GPIO_INPUT:
            GPIO->MODER &= ~(0b11 << 2*pin);
            break;
        case GPIO_OUTPUT:
            GPIO->MODER |= (0b1 << 2*pin);
            GPIO->MODER &= ~(0b1 << (2*pin+1));
            break;
        case GPIO_ALT:
            GPIO->MODER &= ~(0b1 << 2*pin);
            GPIO->MODER |= (0b1 << (2*pin+1));
            break;
        case GPIO_ANALOG:
            GPIO->MODER |= (0b11 << 2*pin);
            break;
    }
}

int digitalRead(int pin) {
    return ((GPIO->IDR) >> pin) & 1;
}

void digitalWrite(int pin, int val) {
    if (val) {
        GPIO->ODR |= (1 << pin);   // Set pin HIGH
        printf("  digitalWrite(%d, %d) - Setting HIGH, ODR before=0x%lX, after=0x%lX\n",
               pin, val, GPIO->ODR & ~(1 << pin), GPIO->ODR);
    } else {
        uint32_t before = GPIO->ODR;
        GPIO->ODR &= ~(1 << pin);  // Set pin LOW
        printf("  digitalWrite(%d, %d) - Setting LOW, ODR before=0x%lX, after=0x%lX\n",
               pin, val, before, GPIO->ODR);
    }
}

void togglePin(int pin) {
    // Use XOR to toggle
    GPIO->ODR ^= (1 << pin);
}