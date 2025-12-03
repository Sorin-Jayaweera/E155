// STM32L432KC_TIM.h
// Header for TIM functions

#ifndef STM32L4_TIM_H
#define STM32L4_TIM_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS device header (provides TIM_TypeDef, TIM15, etc.)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initTIM(TIM_TypeDef * TIMx);
void delay_millis(TIM_TypeDef * TIMx, uint32_t ms);
void initTIM16PWM(void);
void initTIM15Counter(void);
void initTIM16Counter(void);
void setTIM16FREQ(uint32_t freq);
void setTIM15Count(int ms);

#endif