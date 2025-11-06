// hallEncoding.c
// Nina Jobanputra
// njobanputra@hmc.edu
// 10/4/25

#include "hallEncoding.h"
#include <stm32l432xx.h>

// Global variable to keep track of how many interrupts have triggered
int counter = 0;
float PPR   = 408;    // From the motor datasheet

int main(void) {
  // output should be the velocity based on a timer and a counter
  // gpioEnable(GPIO_PORT_A);
  // pinMode(LED_PIN, GPIO_OUTPUT);

  // Enable hall sensor 1 as an input
  gpioEnable(GPIO_PORT_A);
  pinMode(HALL1_PIN, GPIO_INPUT);
  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD1, 0b10);    // Set PA1 as pull-down

  // Enable hall sensor 2 as an input
  gpioEnable(GPIO_PORT_A);
  pinMode(HALL2_PIN, GPIO_INPUT);
  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD2, 0b10);    // Set PA2 as pull-down

  // Initialize timer
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  initTIM(DELAY_TIM);

  // 1. Enable SYSCFG clock domain in RCC
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  // 2. Configure EXTICR for the input button interrupt
  SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI1, 0b000);    // Select PA1
  SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI2, 0b000);    // Select PA2

  // Enable interrupts globally
  __enable_irq();

  // Configure interrupt for falling edge of GPIO pin for button
  // 1. Configure mask bit
  EXTI->IMR1 |= (1 << gpioPinOffset(HALL1_PIN));    // Configure the mask bit
  EXTI->IMR1 |= (1 << gpioPinOffset(HALL2_PIN));    // Configure the mask bit
  // 2. Enable rising edge trigger
  EXTI->RTSR1 |= (1 << gpioPinOffset(HALL1_PIN));    // Enable rising edge trigger
  EXTI->RTSR1 |= (1 << gpioPinOffset(HALL2_PIN));    // Enable rising edge trigger
  // 3. Enable falling edge trigger
  EXTI->FTSR1 |= (1 << gpioPinOffset(HALL1_PIN));    // Enable falling edge trigger
  EXTI->FTSR1 |= (1 << gpioPinOffset(HALL2_PIN));    // Enable falling edge trigger
  // 4. Turn on EXTI interrupt in NVIC_ISER
  NVIC->ISER[0] |= (1 << EXTI1_IRQn);
  NVIC->ISER[0] |= (1 << EXTI2_IRQn);

  while (1) {
    delay_millis(TIM2, 1000);
    computeVel(counter, PPR);
    counter = 0;
    
  }
}

// The IRQHandler for PA8
void EXTI1_IRQHandler(void) {
  // Check that the button was what triggered our interrupt
  if (EXTI->PR1 & (1 << gpioPinOffset(HALL1_PIN))) {
    // If so, clear the interrupt (NB: Write 1 to reset.)
    EXTI->PR1 |= (1 << gpioPinOffset(HALL1_PIN));
    // Keeping track of the direction the motor is going
    if (digitalRead(HALL1_PIN) == 1 && digitalRead(HALL2_PIN) == 0) {
      counter++;
    } else if (digitalRead(HALL1_PIN) == 0 && digitalRead(HALL2_PIN) == 1) {
      counter++;
    } else if (digitalRead(HALL1_PIN) == 0 && digitalRead(HALL2_PIN) == 0) {
      counter--;
    } else if(digitalRead(HALL1_PIN) == 1 && digitalRead(HALL2_PIN) == 1) {
      counter--;
    } else {
      counter = 0;
    }
  }
}

void EXTI2_IRQHandler(void) {
  // Check that the button was what triggered our interrupt
  if (EXTI->PR1 & (1 << gpioPinOffset(HALL2_PIN))) {
    // If so, clear the interrupt (NB: Write 1 to reset.)
    EXTI->PR1 |= (1 << gpioPinOffset(HALL2_PIN));
    // Keeping track of the direction the motor is going
    if (digitalRead(HALL2_PIN) == 1 && digitalRead(HALL1_PIN) == 0) {
      counter--;
    } else if (digitalRead(HALL2_PIN) == 0 && digitalRead(HALL1_PIN) == 1) {
      counter--;
    } else if (digitalRead(HALL2_PIN) == 0 && digitalRead(HALL1_PIN) == 0) {
      counter++;
    } else if (digitalRead(HALL2_PIN) == 1 && digitalRead(HALL1_PIN) == 1){
      counter++;
    } else {
    counter = 0;
    }
  }
}

void computeVel(int counter, float PPR) {
  float vel = (float)counter / (PPR * 4);
  printf("The velocity is %f\n", vel);
}