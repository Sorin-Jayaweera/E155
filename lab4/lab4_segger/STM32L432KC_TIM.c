// 9/29/2025
// Sorin Jayaweera

#include "STM32L432KC_TIM.h"

void initializeTIM15PWM(void){
   //////////////////////////////////////////////////////////////////
   // using TIM15 for driving a pin at pitch frequency
   // PWM MODE (section 28.5.10)
   // This section is enabling PWM
   // change frequency in TIM15_ARR register
   // change duty cycle in TIM15_CCR15 register
   //26.6.1 TIM 15 control register
   //////////////////////////////////////////////////////////////////

    //counter enable
    TIM15->CR1 |= (1<<0);

    //auto reload preload enable
    TIM15->CR1 |= (1<<7);

    // make sure slave mode is DISABLED
    TIM15->SMCR &= ~(1<<7);
   
    // Reset CCMR1 OC1M bits 
    TIM15->CCMR1 &= (0x7 << 4);
    // PWMMODE 1 write 110 to OC1M bits of TIM15_CCMR15 register
    TIM15->CCMR1 |= (0x6 << 4); // 110

    //enable preload register OCxPE bit in TIM15_CCMR1
    TIM15->CCMR1  |= (1 << 3);

    // enable auto-reload preload register for upcounting 
    // set ARPE bit in TIM15_CR1 register
    TIM15->CR1 |= (1 << 7);
  
    // Initialize registers by setting UG bit in TIM15_EGR
    TIM15->EGR |= (1<<0);
    
    //Set to compare mode so that when the cnt hits shadow reg we do smth
    TIM15->CCER |= (1<<0);

    // route with BDTR
    // MOE
    TIM15->BDTR |= (1<<15); 

    // output control bits oc ref + polarity
    // page 938 of reference manual
    // ccer
  
    
    // set pwm to gpio through
    //alternate function
   
}

void initializeTIM15Counter(void){
    ///////////////////////////////////
    // Instead of PWM, just use an upcounter
    // so much easier
    ///////////////////////////////////
    
    //counter enable
    TIM15->CR1 |= (1<<0);

    //auto reload preload enable
    TIM15->CR1 |= (1<<7);

    // make sure slave mode is DISABLED
    TIM15->SMCR &= ~(1<<7);
   

}


void setTIM15FREQ(int freqHz){

  // SET TIM15_ARR 
  //TIM15->ARR = 
  // duty cycle in TIM15_CCR15
  const int TIM16Freq = 9765;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = floor(TIM16Freq/freqHz); // -1 or no?
  
  
  TIM15->PSC = 0; // No division.
  TIM15->ARR = maxcnt;//- 1; //?
  //TIM15->CCR1 = floor(maxcnt/2); // Duty cycle. 50% = 1/2 (ARR+1)

}


  
void initializeTIM16Counter(void){
 //smth so that we have enough bits to 
 //represent a 4 hz signal

 TIM16->PSC = 0;
 //count enable
 TIM16->CR1 |= (1<<0);

 // auto reload preload enable
 TIM16->CR1 |= (1<<7);
  
}

void setTIM16Count(int ms){
  // set the wait time
  
  const int TIM16Freq = 9765;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = floor(TIM16Freq * ms / 1000);
  TIM16->PSC = 0; // No division.
  TIM16-> ARR = maxcnt;   /* Auto-Reload Register        0x2C */
  
}