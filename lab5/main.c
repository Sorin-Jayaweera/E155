//Sorin Jayaweera
//10/4/2025

#include "main.h"

int main(void) {
    // Enable LED as output
    //gpioEnable(GPIO_PORT_B);
    //pinMode(LED_PIN, GPIO_OUTPUT);

    configureFlash();
    configureClock();

    // Enable each reader as input
    gpioEnable(GPIO_PORT_A);
    pinMode(quadencA, GPIO_INPUT);
    pinMode(quadencB, GPIO_INPUT);

    
    //TODO: Find the right GPIO pins for interrupt control
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD4, 0b01); // Set PA4 as pull-up
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD5, 0b01); // Set PA5 as pull-up

    // Initialize timer
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input button interrupt
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI2, 0b000); // Select PA2

    // Enable interrupts globally
    __enable_irq();

    // TODO: Configure interrupt for falling edge of GPIO pin for quadencA
    // 1. Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(quadencA)); // Configure the mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(quadencB)); // Configure the mask bit
    // 2. Disable rising edge trigger
    EXTI->RTSR1 &= ~(1 << gpioPinOffset(quadencA));// Disable rising edge trigger
    EXTI->RTSR1 &= ~(1 << gpioPinOffset(quadencB));// Disable rising edge trigger
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(quadencA));// Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(quadencB));// Enable falling edge trigger
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI2_IRQn); // 2
    NVIC->ISER[0] |= (1 << EXTI3_IRQn); // 3

    while(1){   
        delay_millis(TIM2, 200);
    }

}

void EXTI2_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << )){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << );

    }

    if (EXTI->PR1 & (1 << )){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << );

    }
}