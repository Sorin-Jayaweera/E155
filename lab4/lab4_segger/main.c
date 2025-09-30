//Sorin Jayaweera
// lab 4
// 9/28/2025


// main.c
// GPIO drive a musical note

// Includes for libraries
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_FLASH.h"

// Pitch in Hz, duration in ms
const int notes[][2] = {
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	250},
{  0,	125},
{494,	125},
{523,	125},
{587,	125},
{659,	375},
{392,	125},
{699,	125},
{659,	125},
{587,	375},
{349,	125},
{659,	125},
{587,	125},
{523,	375},
{330,	125},
{587,	125},
{523,	125},
{494,	250},
{  0,	125},
{330,	125},
{659,	125},
{  0,	250},
{659,	125},
{1319,	125},
{  0,	250},
{623,	125},
{659,	125},
{  0,	250},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	500},
{  0,	0}};





// Define macros for constants
#define AUDIO_PIN           3
#define sysclockfreq 80000000 // 80 MHz

#define TIM15_CNTADDR 0x24 // bits 15:0 are counter
#define TIM15_CNTOVF //flag for when the count up triggers


#define TIM16_CNTADDR 0x24 // bits 15:0 are counter


int currentNoteIdx = 0;
bool durationFlag = false;

int main(void) {
    // unbrick the microcontroller
    configureFlash();
    
    //turn on the PLL clock
    // use 80MHz timer 
    configureClock();
    
    // set up clks 15 and 16
    initializeTIM16Counter();
    initializeTIM15PWM();
  

    // Set speaker output as output
    pinMode(AUDIO_PIN, GPIO_OUTPUT);
    

    // Turn on clock to GPIOB
    RCC->AHB2ENR |= (1 << 1);

    // set pwm to gpio through
    //alternate function

    // Clock Configuration Register Handling
    // section 6.4.3

    // Set the frequency of the clock source going into tim15 and tim16
    // the max freq is 2000hz (for ease)
    // min freq is 200 hz 
    // Prescaler1: 512 (max)
    // Prescaler2: 16 (max)
    // this leaves 9765 hz feeding into clk 15 and 16. Those have their own further prescalers

    // enable the clock source for timer 15 and 16
    // TIM15EN address 0x60 + 16   write 1
    RCC->APB2ENR |= (1 << 16);
    // TIM15EN address 0x60 + 17   write 1
    RCC->APB2ENR |= (1 << 17);
    

    // Send clock signal to clk 15 and 16

    // AHB prescaler HPRE:
    // Address 0x08 write 1111 to divide by 512
    RCC->CFGR |= (1111 << 4); //TODO: Or 7?

    // APB Prescaler -> 16
    // Address 0x08 bits 13:11, write 111
    RCC->CFGR |= (111 << 11); //TODO: or 13?

   // we have 16 bits, which is enough to have 0.14 h, aka 6 seconds
   // we don't need any prescaler to make the clock slower


  // song information
  int pitch = notes[i][0]; // hz
  int duration = notes[i][1]; // ms
  
  setTIM15FREQ(pitch);
  setTIM16Count(duration);
  
  while(true){
    durationFlag = 
    while(durationFlag == 0){}
    //UIF bits are interrupt flag
    
    togglePin(pin#)

    
  }



  
   

}


