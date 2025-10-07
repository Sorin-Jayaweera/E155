// button_interrupt.c
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

#include "hallEncoding.h"
#include "main.h"

int main(void) {
    // Enable LED as output
    gpioEnable(GPIO_PORT_A);
    pinMode(LED_PIN, GPIO_OUTPUT);

    // Enable button as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD4, 0b01); // Set PA4 as pull-up

    // Initialize timer
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input button interrupt
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI4, 0b000); // Select PA4

    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for falling edge of GPIO pin for button
    // 1. Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN)); // Configure the mask bit
    // 2. Disable rising edge trigger
    EXTI->RTSR1 &= ~(1 << gpioPinOffset(BUTTON_PIN));// Disable rising edge trigger
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));// Enable falling edge trigger
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI4_IRQn);

    while(1){   
        delay_millis(TIM2, 200);
    }

}

// What is the right name for the IRQHandler?
void EXTI4_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 <<  gpioPinOffset(BUTTON_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 <<  gpioPinOffset(BUTTON_PIN));

        // Then toggle the LED
        togglePin(LED_PIN);

    }
}