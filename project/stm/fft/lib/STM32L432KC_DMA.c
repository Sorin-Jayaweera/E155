// STM32L432KC_DMA.c
// Source code for DMA functions

#include "STM32L432KC_DMA.h"
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_ADC.h"

///////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////

void initDMA_ADC(uint16_t* buffer, uint32_t buffer_size) {
    // Enable DMA1 clock
    RCC->AHB1ENR |= (1 << 0);  // DMA1EN

    // Disable DMA1 Channel 1 before configuration
    DMA1_Channel1->CCR &= ~(1 << 0);  // EN = 0

    // Wait until DMA is disabled
    while (DMA1_Channel1->CCR & (1 << 0));

    // Configure DMA request mapping (ADC1 -> DMA1 Channel 1)
    DMA1_CSELR->CSELR &= ~(0xF << 0);  // Clear C1S bits
    DMA1_CSELR->CSELR |= (DMA_REQUEST_ADC1 << 0);

    // Configure peripheral address (ADC1 data register)
    DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));

    // Configure memory address (buffer)
    DMA1_Channel1->CMAR = (uint32_t)buffer;

    // Configure number of data items to transfer
    DMA1_Channel1->CNDTR = buffer_size;

    // Configure DMA Channel 1
    DMA1_Channel1->CCR = 0;  // Clear all bits first

    // Memory increment mode
    DMA1_Channel1->CCR |= (1 << 7);  // MINC = 1

    // Circular mode
    DMA1_Channel1->CCR |= (1 << 5);  // CIRC = 1

    // Memory size: 16-bit (half-word)
    DMA1_Channel1->CCR |= (DMA_SIZE_16BIT << 10);  // MSIZE

    // Peripheral size: 16-bit (half-word)
    DMA1_Channel1->CCR |= (DMA_SIZE_16BIT << 8);   // PSIZE

    // Priority: Very High
    DMA1_Channel1->CCR |= (DMA_PRIORITY_VERY_HIGH << 12);  // PL

    // Direction: Peripheral to Memory
    DMA1_Channel1->CCR &= ~(1 << 4);  // DIR = 0

    // Enable Transfer Complete interrupt
    DMA1_Channel1->CCR |= (1 << 1);  // TCIE = 1

    // Enable Half Transfer interrupt
    DMA1_Channel1->CCR |= (1 << 2);  // HTIE = 1
}

void enableDMA_ADC(void) {
    // Clear any pending flags
    DMA1->IFCR |= (0xF << 0);  // Clear flags for channel 1

    // Enable DMA1 Channel 1
    DMA1_Channel1->CCR |= (1 << 0);  // EN = 1
}

void disableDMA_ADC(void) {
    // Disable DMA1 Channel 1
    DMA1_Channel1->CCR &= ~(1 << 0);  // EN = 0

    // Wait until disabled
    while (DMA1_Channel1->CCR & (1 << 0));
}

uint32_t getDMA_Counter(void) {
    return DMA1_Channel1->CNDTR;
}
