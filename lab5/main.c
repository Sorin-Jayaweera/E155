//Sorin Jayaweera
//10/4/2025

#include "main.h"

// counts per second
int encCounter = 0;

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
    pinMode(quadencA, GPIO_INPUT);
    pinMode(quadencB, GPIO_INPUT);
    
    //TODO: Find the right GPIO pins for interrupt control
    // 01 is pull up
    // 10 is pull down
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD4, 0b10); // Set PA4 as pull-down
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD5, 0b10); // Set PA5 as pull-down
    
    //// Initialize timer, set it to 1 second always
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);
    setTIMxCount(DELAY_TIM,1000);

    // 1. Enable SYSCFG clock domain in RCC 
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input button interrupt
    SYSCFG->EXTICR[0] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI2, 0b000); // Select PA2
    SYSCFG->EXTICR[0] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI1, 0b000); // Select PA1

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
    NVIC->ISER[0] |= (1 << EXTI1_IRQn); // 2 TURN ON ALL THE EXTI2 INTERRUPTS
    NVIC->ISER[0] |= (1 << EXTI2_IRQn); // 3 TURN ON ALL THE EXTI3 INTERRUPTS
    
    // Enable Software timer 2 interrupt
    NVIC->ISER[0] |= (1<< TIM2_IRQn); // TIM2 is position 28 of the Vector table

    
}

// handle printing every second
void TIM2_IRQHandler(void){
//TODO: THIS TIMER IS TOO FAST
// NVIC is position 28
  if(DELAY_TIM->SR & (1<<0)){ // UIF flag
    int freqHz = ( encCounter) / (24); // 24 edges per cycle
    printf("Speed: %d hz \n", freqHz);
    
    encCounter = 0;
    //DELAY_TIM->SR &= ~(1<<0); //TODO this seems to be rc_w0, but i thought it was rc_w1
    clearTIMx(DELAY_TIM);
  }      
}


// reading the first toggle
void EXTI1_IRQHandler(void){
// NVIC is position 8
    if (EXTI->PR1 & (1 << gpioPinOffset(quadencA))){// PA1
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << gpioPinOffset(quadencA));// clear with 1
        encCounter +=1;
    }
}

// reading the second toggle
void EXTI2_IRQHandler(void){
// NVIC is position 7
    if (EXTI->PR1 & (1 << gpioPinOffset(quadencB))){// Pin A2
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << gpioPinOffset(quadencB));// read clear write 1. 
        encCounter += 1;

    }
}

