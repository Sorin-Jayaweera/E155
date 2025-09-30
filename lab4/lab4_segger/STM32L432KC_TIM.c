#include "STM32L432KC_TIM.h"

void initializeTIM15PWM(){
    //26.6.1 TIM 15 control register

    //counter enable
    TIM15->CR1 |= (1<<0);

    //auto reload preload enable
    TIM15->CR1 |= (1<<7);
    


    
  
   ///////////////////
   // using TIM15 for driving a pin at pitch frequency
   // PWM MODE (section 28.5.10)
   // This section is enabling PWM
   // change frequency in TIM15_ARR register
   // change duty cycle in TIM15_CCR15 register
   
   // PWMMODE 1 write 110 to OCxM bits of TIM15_CCMR15 register

   //enable preload register OCxPE bit in TIM15_CCMR15 

   // enable auto-reload preload register for upcounting 
   // set ARPE bit in TIM15_CR1 register

   // Initialize registers by setting UG bit in TIM15_EGR

  
  
   
}

  
void initializeTIM16Counter(){
 //smth so that we have enough bits to 
 //represent a 4 hz signal
 
  
}

void setTIM15FREQ(int freqHz){

}

void setTIM16Count(int ms){

}