//Sorin Jayaweera
// lab 4
// 9/28/2025

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




// main.c
// GPIO drive a musical note

// Includes for libraries
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432C_FLASH.h"

// Define macros for constants
#define AUDIO_PIN           3
#define sysclockfreq 80000000 // 80 MHz

int main(void) {
    // unbrick the microcontroller
    configureFlash();
    
    //turn on the PLL clock
    // use 80MHz timer 
    configureClock();
    
    // Set speaker output as output
    pinMode(AUDIO_PIN, GPIO_OUTPUT);
    
    // Turn on clock to GPIOB
    RCC->AHB2ENR |= (1 << 1);


    // Clock Configuration Register Handling
    // section 6.4.3

    // enable the clock source for timer 15 and 16
    // TIM15EN address 0x60 + 16   write 1
    RCC->APB2ENR |= (1 << 16);
    // TIM15EN address 0x60 + 17   write 1
    RCC->APB2ENR |= (1 << 17);
    
    
    
    // the max freq is 2000hz (for ease)
    // min freq is 200 hz 
    // Prescaler1: 512 (max)
    // Prescaler2: 16 (max)

    // AHB prescaler HPRE bits 7:
    // Address 0x08 write 1111 to divide by 512
    RCC->CFGR |= (1111 << 4);

    // APB Prescaler -> 16
    // Address 0x08 bits 13:11, write 111
    RCC->CFGR |= (111 << 11);


    int pitchoffset;
    int durationoffset;

    int pitchhightime;
    int notestarttime;
    bool exitflag;


    for(int i = 0; i < sizeof(notes)/size(notes[0]); i++){
        // song information
        int pitch = notes[i][0]; // hz
        int duration = notes[i][1]; // ms

        // for state machine
        exitflag = 0;   

        //setting up counters for pulse time and for freq time
        notestarttime = currenttime;
        pitchhightime = currenttime; 

        // 80 Mhz sysclock
        // cycles/second *second/cycle = number ratio between them to count up to in this divider 
        pitchoffset = sysclockfreq /pitch; // number of time cycles between toggling high or low
        
        // cycles/ second * seconds
        durationoffset = sysclockfreq * duration/1000 ;// number of time cycles until switching to the next note
        
        while(~exitflag){
            currenttime = get_current_time_count();
            // set the frequency for the clk.

            if(currenttime > notestarttime + durationoffset){
                exitflag = 1;
            }
                
            if(curenttime > pitchhightime + pitchoffset){
                //TODO: Potentially divide by 2?
                pitchhightime = currenttime;
                togglePin(AUDIO_PIN);
            }

            
            // do we have to set a case for if the counter wraps around to zero again?
            if(currenttime < notestarttime || currenttime < pitchhightime){
                currenttime = get_current_time_count();
            }
        
        }
         

    }

}

int get_current_time_count(){

}


