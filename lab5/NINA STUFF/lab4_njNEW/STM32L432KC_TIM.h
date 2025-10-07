/*
Author  : Nina Jobanputra
Email   : njobanputra@g.hmc.edu
Date    :  9/30/2025
File    : STM32L432KC_TIM.h
Purpose : header file for setting up timers 15 & 16
*/

#ifndef STM32L4_TIM_H
#define STM32L4_TIM_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions                                                               //
///////////////////////////////////////////////////////////////////////////////

#define __IO volatile

// Base addresses
#define TIM15_BASE (0x40014000UL)
#define TIM16_BASE (0x40014400UL)

typedef struct {
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  uint32_t RESERVED_SMCR;
  __IO uint32_t DIER;
  __IO uint32_t SR;
  __IO uint32_t EGR;
  __IO uint32_t CCMR1;
  uint32_t RESERVED_CCMR1;
  __IO uint32_t CCER;
  __IO uint32_t CNT;
  __IO uint32_t PSC;
  __IO uint32_t ARR;
  __IO uint32_t RCR;
  __IO uint32_t CCR1;
  uint32_t RESERVED_CCR2_0;
  uint32_t RESERVED_CCR2_1;
  uint32_t RESERVED_CCR2_2;
  __IO uint32_t BDTR;
  __IO uint32_t DCR;
  __IO uint32_t DMAR;
  uint32_t OR1;
  uint32_t RESERVED_OR1_0;
  uint32_t RESERVED_OR1_1;
  uint32_t RESERVED_OR1_2;
  __IO uint32_t OR2;
} TIM_TypeDef;

#define TIM15 ((TIM_TypeDef *)TIM15_BASE)
#define TIM16 ((TIM_TypeDef *)TIM16_BASE)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes                                                       //
///////////////////////////////////////////////////////////////////////////////

void init_PWM_TIM(TIM_TypeDef *TIMx);
void init_Delay_TIM(TIM_TypeDef *TIMx);
void delay_millis(TIM_TypeDef *TIMx, uint32_t ms);
void pwm_square(TIM_TypeDef* TIMx, uint32_t freq);

#endif