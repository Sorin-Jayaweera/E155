// STM32L432KC_DMA.h
// Header for DMA functions

#ifndef STM32L4_DMA_H
#define STM32L4_DMA_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define __IO volatile

// Base addresses
#define DMA1_BASE           (0x40020000UL)
#define DMA2_BASE           (0x40020400UL)
#define DMA1_Channel1_BASE  (0x40020008UL)
#define DMA1_Channel2_BASE  (0x4002001CUL)
#define DMA1_Channel3_BASE  (0x40020030UL)
#define DMA1_Channel4_BASE  (0x40020044UL)
#define DMA1_Channel5_BASE  (0x40020058UL)
#define DMA1_Channel6_BASE  (0x4002006CUL)
#define DMA1_Channel7_BASE  (0x40020080UL)

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
// DMA register structures
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    __IO uint32_t CCR;      // DMA channel configuration register
    __IO uint32_t CNDTR;    // DMA channel number of data register
    __IO uint32_t CPAR;     // DMA channel peripheral address register
    __IO uint32_t CMAR;     // DMA channel memory address register
} DMA_Channel_TypeDef;

typedef struct {
    __IO uint32_t ISR;      // DMA interrupt status register,       offset: 0x00
    __IO uint32_t IFCR;     // DMA interrupt flag clear register,   offset: 0x04
} DMA_TypeDef;

typedef struct {
    __IO uint32_t CSELR;    // DMA channel selection register
} DMA_Request_TypeDef;

#define DMA1            ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Channel1   ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2   ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_CSELR      ((DMA_Request_TypeDef *) (DMA1_BASE + 0xA8))

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initDMA_ADC(uint16_t* buffer, uint32_t buffer_size);
void enableDMA_ADC(void);
void disableDMA_ADC(void);
uint32_t getDMA_Counter(void);

#endif
