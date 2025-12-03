// STM32L432KC_DMA.h
// Header for DMA functions

#ifndef STM32L4_DMA_H
#define STM32L4_DMA_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS device header (provides DMA_TypeDef, DMA1, etc.)

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// DMA Request mapping (CSELR register)
#define DMA_REQUEST_ADC1    0

// DMA Priority levels
#define DMA_PRIORITY_LOW        0b00
#define DMA_PRIORITY_MEDIUM     0b01
#define DMA_PRIORITY_HIGH       0b10
#define DMA_PRIORITY_VERY_HIGH  0b11

// DMA Data sizes
#define DMA_SIZE_8BIT   0b00
#define DMA_SIZE_16BIT  0b01
#define DMA_SIZE_32BIT  0b10

// DMA Direction
#define DMA_DIR_PERIPH_TO_MEM   0
#define DMA_DIR_MEM_TO_PERIPH   1

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initDMA_ADC(uint16_t* buffer, uint32_t buffer_size);
void enableDMA_ADC(void);
void disableDMA_ADC(void);
uint32_t getDMA_Counter(void);

#endif
