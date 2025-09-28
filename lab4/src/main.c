// lab4_starter.c
// Fur Elise, E155 Lab 4
// Updated Fall 2024

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

    // enable the clock source for timer 15 and 16
    RCC->APB2ENR |= (1 << 1);

    int currenttime = 0;

    int pitchoffset;
    int durationoffset;

    int pitchhightime;
    int notestarttime;
    bool exitflag;


    for(int i = 0; i < sizeof(nootes)/size(notes[0]); i++){
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
        pitchoffset = sysclockfreq /pitch ; // number of time cycles between toggling high or low
        
        // cycles/ second * seconds
        durationoffset = sysclockfreq * duration/1000 ;// number of time cycles until switching to the next note
        
        while(~exitflag){
            currenttime = get_current_time_count();
            // set the frequency for the clk.

            if(currenttime > notestarttime + durationoffset){
                exitflag = 1;
            }
                
            if(curenttime > pitchhightime + pitchoffset){
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



