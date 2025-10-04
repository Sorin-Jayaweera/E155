//Sorin Jayaweera
//10/4/2025

#include "main.h"

// counts per second
int encCounter = 0;

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

    // use timer 15 for the 1 second timer
    RCC->APB2ENR |= (1 << 16); // tim 15
    initializeTIM15Counter();
    
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


    // hardware interrupt selection 13.3.4 in reference manual
    // 1) configure the mask in the EXTI_IMR register
    // 2) configure trigger selection EXTI_RSTR and eXTI_FTSR
    // 3) Configure the enable and mask bits that control NVIC IRQ channel mapped to the EXTI 
    //    so that an interrupt from EXTI lines can be awknowledged. 

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
    NVIC->ISER[0] |= (1 << EXTI2_IRQn); // 2
    NVIC->ISER[0] |= (1 << EXTI3_IRQn); // 3

    //  software interrupt section 13.3.6 reference manual
    // 1. configure bit mask (EXTI_IMR, EXTI_EMR)
    // 2. Set required bit of software interrupt register EXTI_SWIER
    //
    // 1
    EXTI->IMR1 |= (1<<0); // turn on interrupt zero
    EXTI->EMR1 |= (1<<0); // Event not masked, aka active
    // 2
    // writing 1 sets the corresponding bit in EXTI_PR and makes an interrupt request.
    // Cleared by writing 1 to EXTI_PR
    EXTI->SWIER1  |= (1<<0); 
    
    setTIM15Count(1000);

}

//TODO:
// handle printing every second
void (){ // NOT FOR TIMER INTERRUPT< CHECK DATASHEET. internal!
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << 0)){
        EXTI->PR1 |= (1 << 0); // clear the bit by writing a 1, see 13.5.5 in Ref Manual
        int freqHz = ( encCounter) / (24) //  /2 for avergae, /6 for cycles instead of counts. /2 bc every edge instead of only rising edge
        printf("Speed: %d hz", freqHz);
      
      // TURN OFF THE FLAG FOR TIM15
        encCounter = 0;
    }
}

// reading the first toggle
void EXTI2_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << 2)){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << 2); // clear with 1
        encCounter +=1;
    }
}

// reading the second toggle
void EXTI3_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << 3)){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << 3); // read clear write 1. 
        encCounter += 1;

    }
}

