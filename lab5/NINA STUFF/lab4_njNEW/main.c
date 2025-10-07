/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Allows a speaker to play Fur Elise

*/

// Include for libraries
#include <stdio.h>
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_FLASH.h"

//define macros for constants
#define PWM_PIN           6
//#define DELAY_DURATION_MS    100


// lab4_starter.c
// Fur Elise, E155 Lab 4
// Updated Fall 2024

// Pitch in Hz, duration in ms
const int notes[][2] = {
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	250},
{  0,	125},
{494,	125},
{523,	125},
{587,	125},
{659,	375},
{392,	125},
{699,	125},
{659,	125},
{587,	375},
{349,	125},
{659,	125},
{587,	125},
{523,	375},
{330,	125},
{587,	125},
{523,	125},
{494,	250},
{  0,	125},
{330,	125},
{659,	125},
{  0,	250},
{659,	125},
{1319,	125},
{  0,	250},
{623,	125},
{659,	125},
{  0,	250},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	500},
{  0,	0}};


int main(void) {
    // Configure flash to add waitstates to avoid timing errors
    configureFlash();

    // Setup the PLL and switch clock source to the PLL
    configureClock();

    //set up what you need to enable
    RCC->AHB2ENR |= (1 << 0);       // setting to GPIOA
    RCC->APB2ENR |= (1 << 17);      // enabling timer 16
    RCC->APB2ENR |= (1 << 16);      // enabling timer 15
    
    // configureing pins
    GPIO->AFRL &= ~(0b1111 << 24);
    GPIO->AFRL |= (0b1110 << 24);
    pinMode(PWM_PIN, GPIO_ALT);

    init_Delay_TIM(TIM15);
    init_PWM_TIM(TIM16);

    size_t arrLen = sizeof(notes)/sizeof(notes[0][0]);

    for(size_t i = 0; i <= arrLen; i++) {
     pwm_square(TIM16, notes[i][0]);
     delay_millis(TIM15, notes[i][1]);
     printf("%d\n", i);
    }

    return 0;

}

/*************************** End of file ****************************/
