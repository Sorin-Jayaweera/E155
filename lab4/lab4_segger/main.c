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

//// Pitch in Hz, duration in ms
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
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  {100,	1000},
  //{200,	2000},
  //{300, 3000},
  //{400,	4000},
  //{500,	5000},
  //{600, 6000},
  //{700,	7000},
  //{800,	8000},
  //{900, 9000},
  {0,0}
};

// Define macros for constants
#define AUDIO_PIN         6// 6 //2,3

#ifndef sysclockfreq
  #define sysclockfreq 80000000 // 80 MHz
#endif

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
    // TIM16EN address 0x60 + 16   write 1
    // TIM16EN address 0x60 + 17   write 1
    RCC->APB2ENR |= (1 << 17);
    RCC->APB2ENR |= (1 << 16);

    //RCC->APB2ENR |= (1 << 17);
    // Set the frequency of the clock source going into TIM16 and TIM15
    // the max freq is 2000hz (for ease)
    // min freq is 200 hz 
    // Prescaler1: 512 (max)
    // Prescaler2: 16 (max)
    // this leaves 9765 hz feeding into clk 15 and 16. Those have their own further prescalers
    // AHB prescaler HPRE:
    // Address 0x08 write 1111 to divide by 512
    // RCC->CFGR |= (0x8 << 4);//divide by 2
    // APB Prescaler -> 16
    // Address 0x08 bits 13:11, write 111
    // RCC->CFGR |= (0x8 << 11); // divide by 2
    // we have 16 bits, which is enough to have 0.14 h, aka 6 seconds
    // we don't need to make the clock slower everywhere,
    // lets only do it on the timer15 and 16 side
    /////////////////////////////////////////////////////
    
    RCC->AHB2ENR |= (1 << 0); // GPIO A for PWM connection special functions
    //RCC->AHB2ENR |= (1 << 1); // GPIO B

    printf("RCC APB2ENR %d \n", RCC->APB2ENR);
    printf("RCC CFGR    %d \n", RCC->CFGR);
    printf("RCC AHB2ENR %d \n", RCC->AHB2ENR);
    // set up clks 15 and 16
    initializeTIM15Counter();
    //initializeTIM16Counter();
    initializeTIM16PWM();

    
    // Set speaker output as output
    pinMode(AUDIO_PIN,  GPIO_OUTPUT);//GPIO_ALT);  // alternate function already handles these steps
    /// gpios IN ALTERNATE FUNCTION IN gpioX_moder
    //GPIO->MODER &= ~(0x7 << 2*AUDIO_PIN); //clear
    //GPIO->MODER |=  (0x2 << 2*AUDIO_PIN); // 10

    //// set pin A2 to special function AF14 to be driven by TIM16PWM
    //GPIO->AFRL &= ~(0xF << 4*AUDIO_PIN); // clear
    //GPIO->AFRL |= (0XE << 4*AUDIO_PIN);  //1110 pin 2 alternate function 14 page 272 (ref manual) and 57 (datasheet)
    
    //printf("AFRL: %d \n", GPIO->AFRL);

    int currentNoteIdx = 0;
    int pitch = notes[currentNoteIdx][0]; // hz
    int duration = notes[currentNoteIdx][1]; // ms

    setTIM15Count(duration);
    setTIM16FREQ(pitch);
  
    // manual toggle only works in GPIO_OUTPUT mode. Alternate mode connects it to the timer PWM
    //while(true){
    //  togglePin(AUDIO_PIN);
    //}

    while(true){
      
      printf("Note: %d \n", currentNoteIdx);
      printf("Freq cnt: %d  Duration cnt: %d \n", TIM16->CNT, TIM15->CNT);
      printf("TIM16ARR: %d TIM15ARR: %d \n", TIM16->ARR, TIM15->ARR);
      printf("TIM16PRES: %d TIM15PRES: %d \n", TIM16->PSC, TIM15->PSC);
      printf("TIM15CR1: %d TIM15CR1: %d \n", TIM15->CR1, TIM15->CR1); 
      printf("TIM15SR: %d TIM15SR: %d \n",TIM15->SR,TIM15->SR);

      while(duration == 0 && pitch == 0){}//digitalWrite(AUDIO_PIN, 0);} // end the song

      durationFlagMask = (1<<0);
      durationFlag = (TIM15->SR & durationFlagMask) >> 0;// UIF = Update interrupt flag
      

      freqFlagMask = (1<<0);
      freqFlag = (TIM15->SR & freqFlagMask) >> 0;// UIF = Update interrupt 

      if(durationFlag == 1){

        currentNoteIdx +=1;

        pitch = notes[currentNoteIdx][0]; // hz
        duration = notes[currentNoteIdx][1]; // ms
       
        setTIM15Count(duration);
        setTIM16FREQ(pitch);

        //UIF bits are interrupt flag
        TIM15->SR &= ~(1<<0);// read / clear write 0, turn off the interrupt flag
 
      }


      //HANDLED BY PWM SPECIAL FXN 14
      if(freqFlag == 1){
        if(pitch != 0){ 
          togglePin(AUDIO_PIN); // manually toggle with countup mode, or drive with PWM mode?
          //UIF bits are interrupt flag
          TIM16->SR &= ~(1<<0); // clear the flag
        }
      }
    
    }
}
