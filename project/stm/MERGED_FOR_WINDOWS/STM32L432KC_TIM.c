// STM32L432KC_TIM.c
// TIM functions
// MERGED: DFPlayer base + FFT TIM15/TIM6 support

#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"

///////////////////////////////////////////////////////////////////////////////
// DFPlayer Timer Functions
///////////////////////////////////////////////////////////////////////////////

void initTIM(TIM_TypeDef * TIMx){
  // Set prescaler to give 1 ms time base
  uint32_t psc_div = (uint32_t) ((SystemCoreClock/1e3));

  // Set prescaler division factor
  TIMx->PSC = (psc_div - 1);
  // Generate an update event to update prescaler value
  TIMx->EGR |= 1;
  // Enable counter
  TIMx->CR1 |= 1; // Set CEN = 1
}

void delay_millis(TIM_TypeDef * TIMx, uint32_t ms){
  TIMx->ARR = ms;// Set timer max count
  TIMx->EGR |= 1;     // Force update
  TIMx->SR &= ~(0x1); // Clear UIF
  TIMx->CNT = 0;      // Reset count

  while(!(TIMx->SR & 1)); // Wait for UIF to go high
}

///////////////////////////////////////////////////////////////////////////////
// FFT Timer Functions (added for Tesla coil)
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize TIM6 to trigger ADC at 8 kHz
 *
 * TIMER CALCULATION:
 *   System Clock: 80 MHz (configured by configureClock() in RCC library)
 *   Prescaler: 79 (PSC register, divides by PSC+1 = 80)
 *   Auto-Reload: 124 (ARR register, counts 0 to ARR = 125 counts)
 *   Timer Frequency = 80 MHz / 80 / 125 = 8000 Hz
 *
 * TRIGGER OUTPUT:
 *   CR2.MMS = 010 (Master Mode Selection = Update event)
 *   Generates TRGO signal on each timer update (overflow)
 *   This TRGO connects to ADC1 via EXTSEL = 13 (TIM6_TRGO)
 */
void initTimer_ADC(void) {
    // Enable TIM6 clock (APB1 bus)
    // Bit 4: TIM6EN
    RCC->APB1ENR1 |= (1 << 4);

    // Configure prescaler and auto-reload for 8 kHz
    TIM6->PSC = 79;         // Divide by 80: 80 MHz → 1 MHz
    TIM6->ARR = 125 - 1;    // Count 125 times: 1 MHz → 8 kHz

    // Configure Master Mode Selection (MMS) to output TRGO on update
    // CR2 bits [6:4] = 010 (Update event selected as trigger output)
    TIM6->CR2 = (0x2 << 4);

    // Enable timer (CR1 bit 0: CEN = Counter Enable)
    TIM6->CR1 |= (1 << 0);
}

/**
 * @brief Initialize TIM15 for 100 kHz synthesis interrupt
 *
 * TIMER CALCULATION:
 *   System Clock: 80 MHz
 *   Prescaler: 0 (no division)
 *   Auto-Reload: 799 (counts 0 to 799 = 800 counts)
 *   Timer Frequency = 80 MHz / 1 / 800 = 100 kHz
 */
void initTIM15_Synthesis(void) {
    // Enable TIM15 clock (APB2 bus)
    // Bit 16: TIM15EN
    RCC->APB2ENR |= (1 << 16);

    // Configure for 100 kHz
    TIM15->PSC = 0;         // No division
    TIM15->ARR = 800 - 1;   // 80 MHz / 800 = 100 kHz

    // Enable update interrupt
    // DIER bit 0: UIE = Update Interrupt Enable
    TIM15->DIER |= (1 << 0);

    // Enable TIM15 interrupt in NVIC
    // TIM1_BRK_TIM15 IRQn = 24
    NVIC->ISER[0] |= (1 << 24);

    // Start timer
    // CR1 bit 0: CEN = Counter Enable
    TIM15->CR1 |= (1 << 0);
}
