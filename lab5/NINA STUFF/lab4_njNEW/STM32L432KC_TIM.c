/*
Author  : Nina Jobanputra
Email   : njobanputra@g.hmc.edu
Date    :  9/30/2025
File    : STM32L432KC_TIM.c
Purpose : Source code for TIM functions
*/

#include "STM32L432KC_TIM.h"

/**
 * Sets up a timer for a delay
 * @param TIMx  sets up the registers of a timer
 */
void init_Delay_TIM(TIM_TypeDef* TIMx) {
  // Setting up a timer
  TIMx->PSC = 2000;           // Might want to check this
  TIMx->ARR &= ~(1 << 0);     // Setting ARR to 0
  TIMx->CNT = 0;              // Setting Count to 0
  TIMx->CR1 |= (1 << 0);      // enable counter
  TIMx->EGR |= (1 << 0);      // set bit UG
}

/**
 * Sets up a timer to generates a squarewave for a pitch
 * @param TIMx  sets up the registers of a timer
 */
void init_PWM_TIM(TIM_TypeDef* TIMx) {
  // Setting up a timer
  TIMx->PSC = 55;             // Might want to check this
  TIMx->ARR &= ~(1 << 0);     // Setting ARR to 0

  TIMx->CR1 |= (1 << 7);      // set the ARPE bit to 1
  TIMx->CR1 |= (1 << 0);      // enable counter

  TIMx->CCMR1 |= (1 << 5);    // set the OCxPE bit to 110
  TIMx->CCMR1 |= (1 << 6);    // set the OCxPE bit to 110
  TIMx->CCMR1 &= ~(1 << 4);   // set the OCxPE bit to 110

  TIMx->BDTR |= (1 << 15);    // set the MOE
  TIMx->BDTR |= (1 << 11);    // set the OSSR
  TIMx->BDTR |= (1 <<10);     // set the OSSI

  TIMx->CNT = 0;              // Setting Count to 0
  TIMx->EGR |= (1 << 0);      // set bit UG
  
}

/**
 * Generates a delay
 * @param TIMx  a timer that is being used for the delay
 * @param ms  the amount of time for the delay in milliseconds
 */
void delay_millis(TIM_TypeDef* TIMx, uint32_t ms) {
  // Setting Registers
  int interm = (8*10^6)*TIMx->PSC*1000;
  TIMx->ARR = ms/interm; // Setting the max count of the timer
  TIMx->SR &= ~(0x1);    // Making sure SR is reset
  TIMx->CNT = 0;         // Setting Count to 0
  TIMx->EGR |= (1 << 0); // set bit UG

  // Stay in this while loop while SR is not 1
  while ((TIMx->SR & 0b1) != 1);
}

/**
 * Generates a squarewave for a pitch
 * @param TIMx  a timer that is being used for the wave
 * @param freq  the frequency/ pitch of the note that we want to output
 */
void pwm_square(TIM_TypeDef* TIMx, uint32_t freq) {
  // Setting Registers
  int interm = (8*10^6)/TIMx->PSC;
  TIMx->ARR  = (1/freq) * interm;
  TIMx->CCR1 = (TIMx->ARR) / 2;    // duty cycle
  TIMx->CNT = 0;                   // Setting Count to 0
  TIMx->EGR |= (1 << 0);           // set bit UG
  
  
  //TIMx->CCER &= ~(1 << 1);       // set the CCxP
  //TIMx->CCER |= (1 << 0);        // set the CCxE
  //TIMx->CCER;                    // set the CCxNE
}

