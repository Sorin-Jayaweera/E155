// STM32F401RE_TIM.c
// TIM functions

#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"

void initTIM(TIM_TypeDef * TIMx){
  // Set prescaler to give 1 ms time base
  uint32_t psc_div = 8000;//(uint32_t) ((SystemCoreClock/1e3));
  
  // Set prescaler division factor
  TIMx->PSC = (psc_div - 1);
  // Generate an update event to update prescaler value
  TIMx->EGR |= 1;
  // Enable counter
  TIMx->CR1 |= 1; // Set CEN = 1

  // set timer to have interrupts
  TIMx->DIER |= (1<<6);// Trigger interrupt enable
  TIMx->DIER |= (1<<0);// Trigger interrupt enable
}

void delay_millis(TIM_TypeDef * TIMx, uint32_t ms){
  TIMx->ARR = ms;// Set timer max count
  TIMx->EGR |= 1;     // Force update
  TIMx->SR &= ~(0x1); // Clear UIF
  TIMx->CNT = 0;      // Reset count

  while(!(TIMx->SR & 1)); // Wait for UIF to go high
}


void setTIMxCount(TIM_TypeDef * TIMx, uint32_t ms){
  // set the wait time
  
  const int TIMFreq = 10000;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = ceil(TIMFreq * ms / 1000)-1; // cycles / second * seconds = cycles

  TIMx->ARR = maxcnt;   /* Auto-Reload Register        0x2C */
  TIMx->EGR |= (0b1<<0); // force things to update by writing UG bit to 0
  TIMx->SR &= ~(1<<0);
  TIMx->CNT = 0;     

}

void clearTIMx(TIM_TypeDef * TIMx){
  TIMx->EGR |= 1;     // Force update
  TIMx->SR &= ~(0x1); // Clear UIF
  TIMx->CNT = 0;     
}


// 9/29/2025
// Sorin Jayaweera

#include "STM32L432KC_TIM.h"
#ifndef sysclockfreq
  #define sysclockfreq 20000000 // 80 MHz
#endif

#ifndef STM32L4_RCC_H
#include "STM32L432KC_RCC.h"
#endif


  
void initializeTIM15Counter(void){
 //smth so that we have enough bits to 
 //represent a 4 hz signalD
 // clear
 //TIM15->CR1 &= ~(0xF);

 //set prescaler = 2^13
  TIM15->PSC = 7999;//8000;// 10000 hz, which needs 13 bits for one second - so we have 8 seconds max

 //turn off UDIS bit
 TIM15->CR1 &= ~(0b1<<1);

 // auto reload preload enable
 TIM15->CR1 |= (0b1<<7);

 //TODO: feels wrong
 //set an initial thing to count up to
 //TIM15->ARR |= (0b1<<4);

 //clear flag
 TIM15->EGR &= ~(0b1<<0);
 
 //count enable
 TIM15->CR1 |= (0b1<<0);
}



void setTIM15Count(int ms){
  // set the wait time
  
  const int TIM15Freq = 10000;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = ceil(TIM15Freq * ms / 1000)-1; // cycles / second * seconds = cycles

  //TIM15->PSC = 0; // No division.
  TIM15->ARR = maxcnt;   /* Auto-Reload Register        0x2C */
  TIM15->EGR |= (0b1<<0); // force things to update by writing UG bit to 0
  TIM15->SR &= ~(1<<0);
}