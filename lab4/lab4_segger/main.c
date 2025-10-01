//Sorin Jayaweera
// lab 4
// 9/28/2025
// FOLDER:
//C:\ThatFamily Dropbox\Sorin Jayaweera\allSaves\mudd\junior\MicroPs

// main.c
// GPIO drive a musical note

// Standard libraries
#include <stdbool.h>
#include <stdio.h>

// Includes for libraries
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_FLASH.h"

////// Pitch in Hz, duration in ms
//const int notes[][2] = {
//{659,	125},
//{623,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{494,	125},
//{587,	125},
//{523,	125},
//{440,	250},
//{  0,	125},
//{262,	125},
//{330,	125},
//{440,	125},
//{494,	250},
//{  0,	125},
//{330,	125},
//{416,	125},
//{494,	125},
//{523,	250},
//{  0,	125},
//{330,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{494,	125},
//{587,	125},
//{523,	125},
//{440,	250},
//{  0,	125},
//{262,	125},
//{330,	125},
//{440,	125},
//{494,	250},
//{  0,	125},
//{330,	125},
//{523,	125},
//{494,	125},
//{440,	250},
//{  0,	125},
//{494,	125},
//{523,	125},
//{587,	125},
//{659,	375},
//{392,	125},
//{699,	125},
//{659,	125},
//{587,	375},
//{349,	125},
//{659,	125},
//{587,	125},
//{523,	375},
//{330,	125},
//{587,	125},
//{523,	125},
//{494,	250},
//{  0,	125},
//{330,	125},
//{659,	125},
//{  0,	250},
//{659,	125},
//{1319,	125},
//{  0,	250},
//{623,	125},
//{659,	125},
//{  0,	250},
//{623,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{494,	125},
//{587,	125},
//{523,	125},
//{440,	250},
//{  0,	125},
//{262,	125},
//{330,	125},
//{440,	125},
//{494,	250},
//{  0,	125},
//{330,	125},
//{416,	125},
//{494,	125},
//{523,	250},
//{  0,	125},
//{330,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{623,	125},
//{659,	125},
//{494,	125},
//{587,	125},
//{523,	125},
//{440,	250},
//{  0,	125},
//{262,	125},
//{330,	125},
//{440,	125},
//{494,	250},
//{  0,	125},
//{330,	125},
//{523,	125},
//{494,	125},
//{440,	500},
//{  0,	0}};


const int notes[][2] = {
 {500, 1002},
 {200, 1002},
 {300, 1192},
 {400, 1002},
 {500, 1002},
 {600, 1002},
 {700, 1002},
 {800, 1002},
 {900, 1002},
 {100, 1002},
 {200, 1002},
 {300, 1002},
 {400, 1002},
 {500, 1002},
 {600, 1002},
 {700, 1002},
 {800, 1002},
 {900, 1002},
 {100, 1002},
 {200, 1002},
 {300, 1002},
 {400, 1002},
 {500, 1002},
 {600, 1002},
 {700, 1002},
 {800, 1002},
 {900, 1002},
  {0,	0}
};



// Define macros for constants
#define AUDIO_PIN          2 //3 //6 //A6

#ifndef sysclockfreq
  #define sysclockfreq 80000000 // 80 MHz
#endif

#define TIM15_CNTADDR 0x24 // bits 15:0 are counter
#define TIM15_CNTOVF //flag for when the count up triggers
#define TIM16_CNTADDR 0x24 // bits 15:0 are counter

uint8_t durationFlagMask;  
uint8_t freqFlagMask;
bool durationFlag;
bool freqFlag;


int main(void) {
    // unbrick the microcontroller
    configureFlash();
    
    //turn on the PLL clock
    // use 80MHz timer 
    configureClock();
    
    /////////////////////////////////////////////////////
    // Clock Configuration Register Handling
    // section 6.4.3
    // enable the clock source for timer 15 and 16
    // TIM15EN address 0x60 + 16   write 1
    // TIM15EN address 0x60 + 17   write 1
    RCC->APB2ENR |= (1 << 17);
    RCC->APB2ENR |= (1 << 16);

    //RCC->APB2ENR |= (1 << 17);
    // Set the frequency of the clock source going into tim15 and tim16
    // the max freq is 2000hz (for ease)
    // min freq is 200 hz 
    // Prescaler1: 512 (max)
    // Prescaler2: 16 (max)
    // this leaves 9765 hz feeding into clk 15 and 16. Those have their own further prescalers
    // AHB prescaler HPRE:
    // Address 0x08 write 1111 to divide by 512
    RCC->CFGR |= (0xF << 4);//1111
    // APB Prescaler -> 16
    // Address 0x08 bits 13:11, write 111
    RCC->CFGR |= (0x7 << 11); // 111
    // we have 16 bits, which is enough to have 0.14 h, aka 6 seconds
    // we don't need any prescaler to make the clock slower
    /////////////////////////////////////////////////////
    
    RCC->AHB2ENR |= (1 << 0); // GPIO A for PWM connection special functions
    //RCC->AHB2ENR |= (1 << 1); // GPIO B

    printf("RCC APB2ENR %d \n", RCC->APB2ENR);
    printf("RCC CFGR    %d \n", RCC->CFGR);
    printf("RCC AHB2ENR %d \n", RCC->AHB2ENR);
    // set up clks 15 and 16
    initializeTIM16Counter();
    //initializeTIM15Counter();
    initializeTIM15PWM();

    
    // Set speaker output as output
    pinMode(AUDIO_PIN, GPIO_ALT); // GPIO_OUTPUT
  
    // set pin A2 to special function AF14 to be driven by TIM15PWM
    GPIO->AFRL &= ~(0xF << 8); // clear
    GPIO->AFRL |= (0XE << 8);  //1110 pin 2 alternate function 14 page 272 (ref manual) and 57 (datasheet)

    int currentNoteIdx = 0;
    int pitch = notes[currentNoteIdx][0]; // hz
    int duration = notes[currentNoteIdx][1]; // ms
    setTIM16Count(duration);
    setTIM15FREQ(pitch);
   
   // outputting to pin working
   // freq too slow
   // while(true){ // debug testing output
   //     ms_delay(DELAY_DURATION_MS);
   //     togglePin(AUDIO_PIN);
   //}


    while(true){
      
      printf("Note: %d \n", currentNoteIdx);
      printf("Duration cnt: %d Freq cnt: %d \n", TIM15->CNT, TIM16->CNT);
      printf("TIM15ARR: %d TIM16ARR: %d \n", TIM15->ARR, TIM16->ARR);
      printf("TIM15PRES: %d TIM16PRES: %d \n", TIM15->PSC, TIM16->PSC);
      printf("TIM15CR1: %d TIM16CR1: %d \n", TIM15->CR1, TIM16->CR1); 
      printf("TIM15SR: %d TIM16SR: %d \n",TIM15->SR,TIM16->SR);

      while(duration == 0 && pitch == 0){}//digitalWrite(AUDIO_PIN, 0);} // end the song

      durationFlagMask = (1<<0);
      durationFlag = (TIM16->SR & durationFlagMask) >> 0;// UIF = Update interrupt flag
      

      //freqFlagMask = (1<<0);
      //freqFlag = (TIM15->SR & freqFlagMask) >> 0;// UIF = Update interrupt 

      if(durationFlag == 1){

        currentNoteIdx +=1;

        pitch = notes[currentNoteIdx][0]; // hz
        duration = notes[currentNoteIdx][1]; // ms
      
        setTIM16Count(duration);
        setTIM15FREQ(pitch);

        //UIF bits are interrupt flag
        TIM16->SR &= ~(1<<0);// read / clear write 0, turn off the interrupt flag
 
      }
      if(durationFlag == 0){
      printf("PLEASE");
      }

    // HANDLED BY PWM SPECIAL FXN 14
      if(freqFlag == 1){
        if(pitch != 0){ 
          togglePin(AUDIO_PIN); // manually toggle with countup mode, or drive with PWM mode?
          //UIF bits are interrupt flag
          TIM15->SR &= ~(1<<0); // clear the flag
        }
      }
    
    }
}
