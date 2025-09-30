// 9/29/2025
// Sorin Jayaweera

#include "STM32L432KC_TIM.h"

void initializeTIM15PWM(){
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
   

    // PWMMODE 1 write 110 to OC1M bits of TIM15_CCMR15 register
    TIM15->CCMR1 |= (110 << 4);

    //enable preload register OCxPE bit in TIM15_CCMR1
    TIM15->CCMR1  |= (1 << 3);

    // enable auto-reload preload register for upcounting 
    // set ARPE bit in TIM15_CR1 register
    TIM15->CR1 |= (1 << 7);
  
    // Initialize registers by setting UG bit in TIM15_EGR
    TIM15->EGR |= (1<<0);
  
    // route with BDTR
    // MOE
    TIM15->BDTR |= (1<<15); //TODO: DO I NEED THIS?
    
    // output control bits oc ref + polarity
    // page 938 of reference manual
    // ccer
  
    
    // set pwm to gpio through
    //alternate function
   
}

void initializeTIM15Counter(){
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
  uint16_t maxcnt = floor(TIM16Freq/freqHz);
  TIM15->ARR = maxcnt;


}


  
void initializeTIM16Counter(){
 //smth so that we have enough bits to 
 //represent a 4 hz signal

 //count enable
 TIM16->CR1 |= (1<<0);

 // auto reload preload enable
 TIM16->CR1 |= (1<<7);
  
}

void setTIM16Count(int ms){
  // set the 
  const int TIM16Freq = 9765;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = floor(TIM16Freq * ms / 1000);
  TIM16-> ARR = maxcnt;   /* Auto-Reload Register        0x2C */
  
}