// 9/29/2025
// Sorin Jayaweera

#include "STM32L432KC_TIM.h"
#ifndef sysclockfreq
  #define sysclockfreq 20000000 // 80 MHz
#endif
void initializeTIM15PWM(void){
   //////////////////////////////////////////////////////////////////
   // using TIM15 for driving a pin at pitch frequency
   // PWM MODE (section 28.5.10)
   // This section is enabling PWM
   // change frequency in TIM15_ARR register
   // change duty cycle in TIM15_CCR15 register
   // 26.6.1 TIM 15 control register
   //////////////////////////////////////////////////////////////////



    ////auto reload preload enable
    //TIM15->CR1 |= (0b1<<7);
    

    //// make sure slave mode is DISABLED MSM
    //TIM15->SMCR &= ~(0b1<<7);

    //// PWMMODE 1 write 110 to OC1M bits of TIM15_CCMR1 register
    //TIM15->CCMR1 |= (0x7 << 4); //clear
    //TIM15->CCMR1 |= (0x6 << 4); //set 110 PWM mode

    //// What about 011, 28.5.9 toggle
    
    //// Reset CCMR1 OC2M bits 
    ////TIM15->CCMR1 &= ~(0x7 << 12); //clear
    ////TIM15->CCMR1 |= (0x6 << 12); // 0110 PWM mode


    ////enable preload register OCxPE bit in TIM15_CCMR1
    //TIM15->CCMR1  |= (0b1 << 3); // OC1PE
    ////TIM15->CCMR1  |= (0b1 << 11); // OC2PE

    //// enable auto-reload preload register for upcounting 
    //// set ARPE bit in TIM15_CR1 register
    //TIM15->CR1 |= (0b1 << 7); // ARPR
  
    //// Initialize registers by setting UG bit in TIM15_EGR
    //TIM15->EGR |= (0b1<<0); // IRQ FLAG
    
    ////Set to compare mode so that when the cnt hits shadow reg we do smth'
    //// corresponding output pin depending on MOE, OSSI, OSSR, OIS1, OIS1N and CC1NE bits
    //TIM15->CCER |= (0b1<<0); 

    //// route with BDTR
    //// MOE
    //TIM15->BDTR |= (0b1<<15); 

    //// want OCxREF + polarity
    ////ossr bit 0
    //TIM15->BDTR &= ~(0b1 << 11);

    ////ccxE bit 1
    //TIM15->CCER |= (0b1 << 0); // CC1E
    //TIM15->CCER |= (0b1 << 5); // CC2E

    ////CCxNE bit 0
    //TIM15->CCER &= ~(0b1 << 2);  // CC1NE
    //TIM15->CCER &= ~(0b1 << 2);  // CC2NE

    //// set preload register 
    //TIM15->CCMR1 |= (0b1<<3); //OC1PE

    ////counter enable
    //TIM15->CR1 |= (0b1<<0);

    ////back to page 748

    //// set pwm to gpio through
    ////alternate function
   /////////////////////////////////////////////////////////


  // try two, from scratch
  // using pwm channel 1
  TIM15->PSC = 800; // 100000 hz
  // set PWM mode 1
  //OC1M to 110 in TIM15_CCMR1
  TIM15->CCMR1 &= ~(0xF << 4);
  TIM15->CCMR1 |= (0b110 << 4);

  //enable preload register
  // OC1PE in TIM15_CCMR1
  TIM15->CCMR1 |= (0b1 << 3);

  // set auto reload preload register in upcounting
  // APRE bit in TIM15_CR1
  TIM15->CR1 |= (0b1<<7); // TIMx_ARR register is buffered

  //TODO: IFFY, based off my own thoughts
  // done because of description for CC1P bit
  //TIM15->DIER |= (1<<1); // CC1IE interrupt enable.

  // OC1 polarity 
  // use CC1P bit in TIM15_CCER register, active high
  //TIM15->CCER |= (0b1 <<1);

  // OC1 output enabled by CC1E, CC1NE, MOE, OSSI, OSSR (TIM15_CCER and TIM15_BDTR registers)
  //  sig out on pin depending on MOE, OSSI, OSSR, OIS1, OIS1N and CC1NE bits.
  TIM15->CCER |= (1 << 0); 

  // MOE
  TIM15->BDTR |= (1<<15);

  //OSSI: not needed
  //TIM15->BDTR |= (1<<10);
  
  //OSSR: 0
  TIM15->BDTR &= ~(1<<11);

  // CC1E: 1
  TIM15->CCER |= (1 << 0); 

  //CC1NE 0
  TIM15->CCER &= ~(1 << 2); 

  // Initialize all registers by setting UG bit in TIM15_EGR register
  TIM15->EGR |= (1<<0); // in instructions, this was earlier/ See page 906 of ref manual.

  // set TIM15_CCR1 to be half of TIM15_ARR (TODO CHECK if it is arr)
  TIM15->CR1 |= (1<<0);



  ///////////////////
  // from section 28.5.9 Output compare mode

  // Select counter clock (internal, external, prescaler
   
  // Write Data in TIM15_ARR and TIM15CCR

  //don't care about interrupt request, but if you do then set CC1IE bit

  //Select output mode
  //OC1M = 011 to toggle on pwm

  // Enable the counter by setting CEN in TIM15_CR1 reg


}

//void initializeTIM15Counter(void){
//    ///////////////////////////////////
//    // Instead of PWM, just use an upcounter
//    // so much easier
//    ///////////////////////////////////
    
//    // clear
//    //TIM15->CR1 &= ~(0xF);

//    //set prescaler = 200
//    TIM15->PSC &= ~(0xF);
//    TIM15->PSC = 200; //100000 hz

//    //turn off UDIS bit
//    TIM15->CR1 &= ~(0b1<<1);

//    //auto reload preload enable
//    TIM15->CR1 |= (0b1<<7);

//    // make sure slave mode is DISABLED
//    TIM15->SMCR &= ~(0b1<<7);

//    //clear flag
//    TIM15->EGR &= ~(0b1<<0);
      
//    //counter enable
//    TIM15->CR1 |= (0b1<<0);
//}


  
void initializeTIM16Counter(void){
 //smth so that we have enough bits to 
 //represent a 4 hz signal
  
 // clear
 //TIM16->CR1 &= ~(0xF);

 //set prescaler = 1
  TIM16->PSC = 8000;// 20000 hz

 //turn off UDIS bit
 TIM16->CR1 &= ~(0b1<<1);

 // auto reload preload enable
 TIM16->CR1 |= (0b1<<7);

 //TODO: feels wrong
 //set an initial thing to count up to
 TIM16->ARR |= (0b1<<4);

 //clear flag
 TIM16->EGR &= ~(0b1<<0);
 
 //count enable
 TIM16->CR1 |= (0b1<<0);
}


void setTIM15FREQ(int freqHz){

  // SET TIM15_ARR 
  //TIM15->ARR = 
  // duty cycle in TIM15_CCR15
  if(freqHz != 0){
  
    const int TIM15Freq = 100000;//hz. cycles/sec Calculated by: 80 Mhz / 
    uint16_t maxcnt = ceil(TIM15Freq/freqHz) -1; // -1 or no?

    //TIM15->PSC = 0; // freq/ (num + 1) -> No division.
    TIM15->ARR = maxcnt;// on reload new count register
    TIM15->CCR1 = ceil(maxcnt/2); // Duty cycle 50% = 1/2 (ARR+1)
    TIM15->EGR |= (1<<0); 
    TIM15->CNT = 0;
    //
  }

}

void setTIM16Count(int ms){
  // set the wait time
  
  const int TIM16Freq = 10000;//hz. cycles/sec Calculated by: 80 Mhz / (512*16)
  uint16_t maxcnt = ceil(TIM16Freq * ms / 1000); // cycles / second * seconds = cycles

  //TIM16->PSC = 0; // No division.
  TIM16->ARR = maxcnt;   /* Auto-Reload Register        0x2C */
  TIM16->EGR |= (0b1<<0); // force things to update by writing UG bit to 0
  TIM16->SR &= ~(1<<0);
}