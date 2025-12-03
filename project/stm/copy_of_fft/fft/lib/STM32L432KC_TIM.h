// STM32L432KC_TIM.h
// Header for TIM functions
// MERGED: DFPlayer base + FFT TIM15/TIM6 support

#ifndef STM32L4_TIM_H
#define STM32L4_TIM_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS device library include
#include "STM32L432KC_GPIO.h"

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

// DFPlayer Timer Functions
void initTIM(TIM_TypeDef * TIMx);
void delay_millis(TIM_TypeDef * TIMx, uint32_t ms);

// FFT Timer Functions (added for Tesla coil synthesis)
void initTimer_ADC(void);         // Initialize TIM6 for 8 kHz ADC trigger
void initTIM15_Synthesis(void);   // Initialize TIM15 for 100 kHz synthesis

#endif
