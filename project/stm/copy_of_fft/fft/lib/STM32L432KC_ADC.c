// STM32L432KC_ADC.c
// Source code for ADC functions

#include "STM32L432KC_ADC.h"
#include "STM32L432KC_RCC.h"

///////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////

void calibrateADC(void) {
    // Ensure ADC is disabled
    if (ADC1->CR & (1 << 0)) {  // ADEN bit
        ADC1->CR |= (1 << 1);   // ADDIS bit - disable ADC
        while (ADC1->CR & (1 << 1));  // Wait until disabled
    }

    // Start calibration
    ADC1->CR &= ~(1 << 30);  // ADCALDIF = 0 (single-ended calibration)
    ADC1->CR |= (1 << 31);   // ADCAL = 1 (start calibration)

    // Wait for calibration to complete
    while (ADC1->CR & (1 << 31));
}

void initADC(uint8_t channel) {
    // Enable ADC clock
    RCC->AHB2ENR |= (1 << 13);  // Enable ADC clock

    // Exit deep power down
    ADC1->CR &= ~(1 << 29);  // DEEPPWD = 0

    // Enable internal voltage regulator
    ADC1->CR |= (1 << 28);  // ADVREGEN = 1

    // Wait for regulator to start up (tADCVREG_STUP = 20 us)
    // At 80 MHz, need ~1600 cycles
    for (volatile int i = 0; i < 2000; i++);

    // Calibrate ADC
    calibrateADC();

    // Configure ADC clock - use synchronous clock div by 1 (HCLK/1)
    ADC_COMMON->CCR &= ~(0b11 << 16);  // Clear CKMODE
    ADC_COMMON->CCR |= (ADC_CLOCK_SYNC_DIV1 << 16);

    // Configure resolution to 12-bit
    ADC1->CFGR &= ~(0b11 << 3);  // Clear RES bits
    ADC1->CFGR |= (ADC_RES_12BIT << 3);

    // Configure continuous conversion mode
    ADC1->CFGR |= (1 << 13);  // CONT = 1

    // Configure overrun mode - overwrite old data
    ADC1->CFGR |= (1 << 12);  // OVRMOD = 1

    // Set sampling time for the selected channel (6.5 ADC clock cycles)
    if (channel < 10) {
        ADC1->SMPR1 &= ~(0b111 << (channel * 3));
        ADC1->SMPR1 |= (ADC_SMPTIME_6_5 << (channel * 3));
    } else {
        ADC1->SMPR2 &= ~(0b111 << ((channel - 10) * 3));
        ADC1->SMPR2 |= (ADC_SMPTIME_6_5 << ((channel - 10) * 3));
    }

    // Configure sequence - single channel
    ADC1->SQR1 &= ~(0b1111 << 0);  // L = 0 (1 conversion)
    ADC1->SQR1 &= ~(0b11111 << 6); // Clear SQ1
    ADC1->SQR1 |= (channel << 6);  // Set channel

    // Enable ADC
    ADC1->ISR |= (1 << 0);  // Clear ADRDY flag
    ADC1->CR |= (1 << 0);   // ADEN = 1
    while (!(ADC1->ISR & (1 << 0)));  // Wait for ADRDY
}

void configureADCForDMA(uint8_t channel) {
    // Enable ADC clock
    RCC->AHB2ENR |= (1 << 13);  // Enable ADC clock

    // Exit deep power down
    ADC1->CR &= ~(1 << 29);  // DEEPPWD = 0

    // Enable internal voltage regulator
    ADC1->CR |= (1 << 28);  // ADVREGEN = 1

    // Wait for regulator to start up
    for (volatile int i = 0; i < 2000; i++);

    // Calibrate ADC
    calibrateADC();

    // Configure ADC clock - use synchronous clock div by 1 for max speed
    ADC_COMMON->CCR &= ~(0b11 << 16);  // Clear CKMODE
    ADC_COMMON->CCR |= (ADC_CLOCK_SYNC_DIV1 << 16);

    // Configure resolution to 12-bit
    ADC1->CFGR &= ~(0b11 << 3);  // Clear RES bits
    ADC1->CFGR |= (ADC_RES_12BIT << 3);

    // Configure continuous conversion mode
    ADC1->CFGR |= (1 << 13);  // CONT = 1

    // Enable DMA
    ADC1->CFGR |= (1 << 0);   // DMAEN = 1
    ADC1->CFGR |= (1 << 1);   // DMACFG = 1 (circular mode)

    // Configure overrun mode
    ADC1->CFGR |= (1 << 12);  // OVRMOD = 1

    // Set sampling time (fast: 2.5 cycles for high-speed audio sampling)
    if (channel < 10) {
        ADC1->SMPR1 &= ~(0b111 << (channel * 3));
        ADC1->SMPR1 |= (ADC_SMPTIME_2_5 << (channel * 3));
    } else {
        ADC1->SMPR2 &= ~(0b111 << ((channel - 10) * 3));
        ADC1->SMPR2 |= (ADC_SMPTIME_2_5 << ((channel - 10) * 3));
    }

    // Configure sequence - single channel
    ADC1->SQR1 &= ~(0b1111 << 0);  // L = 0 (1 conversion)
    ADC1->SQR1 &= ~(0b11111 << 6); // Clear SQ1
    ADC1->SQR1 |= (channel << 6);  // Set channel

    // Enable ADC
    ADC1->ISR |= (1 << 0);  // Clear ADRDY flag
    ADC1->CR |= (1 << 0);   // ADEN = 1
    while (!(ADC1->ISR & (1 << 0)));  // Wait for ADRDY
}

void startADC(void) {
    // Start conversion
    ADC1->CR |= (1 << 2);  // ADSTART = 1
}

void stopADC(void) {
    // Stop conversion
    ADC1->CR |= (1 << 4);  // ADSTP = 1
    while (ADC1->CR & (1 << 4));  // Wait for stop
}

uint16_t readADC(void) {
    // Wait for end of conversion
    while (!(ADC1->ISR & (1 << 2)));  // Wait for EOC

    // Read data
    return (uint16_t)(ADC1->DR & 0xFFFF);
}
