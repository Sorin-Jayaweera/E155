//Sorin Jayaweera
//10/4/2025

#include "main.h"
#include <stdbool.h>
#include <stdio.h>

// counts per second
int encCounter = 0;
bool polarity = false;
bool lastToggle = 0;

int main(void) {
    
    // Configure 
    // Flash and Clock to unbrick
    // 80 MHz PLL for system clock
    // enable clock 15 for 1 second timer
    // turn two pins for reading the encoders to input
    // #################################################################
    configureFlash();
    configureClock();

    // GPIO
    // Enable each reader as input
    gpioEnable(GPIO_PORT_A);
    //    gpioEnable(GPIO_PORT_B); // in sample code was B
    pinMode(quadencA, GPIO_INPUT);
    pinMode(quadencB, GPIO_INPUT);
    
    // 01 is pull up
    // 10 is pull down
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD1, 0b10); // Set PA1 as pull-down
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD2, 0b10); // Set PA2 as pull-down
    
    //// Initialize timer, set it to 1 second always
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);
    setTIMxCount(DELAY_TIM,1000);

    
    
    // 1. Enable SYSCFG clock domain in RCC 
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input button interrupt


    //TODO: Change to 0 or 1 for ICR maybe? ~Nina
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI1, 0b000); // Select PA1
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI2, 0b000); // Select PA2

    // Enable interrupts globally
    __enable_irq(); // 4.3.10 of programmers manual

    // hardware interrupt selection 13.3.4 in reference manual
    // 1) configure the mask in the EXTI_IMR register
    // 2) configure trigger selection EXTI_RSTR and eXTI_FTSR
    // 3) Configure the enable and mask bits that control NVIC IRQ channel mapped to the EXTI 
    //    so that an interrupt from EXTI lines can be awknowledged. 
    // #################################################################
    // 1. Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(quadencA)); // Configure the mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(quadencB)); // Configure the mask bit
    // 2. Enable rising edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(quadencA));// Disable rising edge trigger // ON EVERY EDGE
    EXTI->RTSR1 |= (1 << gpioPinOffset(quadencB));// Disable rising edge trigger
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(quadencA));// Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(quadencB));// Enable falling edge trigger
    // 4. Turn on EXTI interrupt in NVIC_ISER
  
    NVIC->ISER[0] |= (1 << EXTI1_IRQn); // TURN ON ALL THE EXTI1 INTERRUPTS
    NVIC->ISER[0] |= (1 << EXTI2_IRQn); // TURN ON ALL THE EXTI2 INTERRUPTS
    
    // Enable Software timer 2 interrupt
    NVIC->ISER[0] |= (1<< TIM2_IRQn); // TIM2 is position 28 of the Vector 


    //while(true){};
}

float freqHz;
// handle printing every second
void TIM2_IRQHandler(void){
// NVIC is position 28
  if(DELAY_TIM->SR & (1<<0)){ // UIF flag
    const float ppr = 1632;
    freqHz = ((float)encCounter)/ppr;//408*2*2); 
    printf(" Speed: %f hz ", freqHz);
    printf(" polarity: %d \n", polarity);
    
    encCounter = 0;
    //DELAY_TIM->SR &= ~(1<<0); //TODO this seems to be rc_w0, but i thought it was rc_w1
    clearTIMx(DELAY_TIM);
  }      
}

// reading the first toggle
void EXTI1_IRQHandler(void){
// NVIC is position 8
    if (EXTI->PR1 & (1 << gpioPinOffset(quadencA))){// PA1 activated interrupt
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << gpioPinOffset(quadencA));// clear with 1
        encCounter +=1;
        if(lastToggle == 1){
          polarity = !polarity;
        }
        lastToggle = 1;
    }
}

// reading the second toggle
void EXTI2_IRQHandler(void){
    // NVIC is position 7
    if (EXTI->PR1 & (1 << gpioPinOffset(quadencB))){// Pin A2
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << gpioPinOffset(quadencB));// read clear write 1. 
        encCounter += 1;
        if(lastToggle == 0){
          polarity = !polarity;
        }
        lastToggle = 0;
    }
}

