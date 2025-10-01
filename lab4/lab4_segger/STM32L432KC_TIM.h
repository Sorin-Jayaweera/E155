// STM32L432KC_RCC.h
// Header for RCC functions

#ifndef STM32L4_TIM_H
#define STM32L4_TIM_H

#include <stdint.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define __IO volatile

// Base addresses
#define TIM15_BASE (0x40014000UL) // base address of TIM15
#define TIM16_BASE (0x40014400UL) // base address of TIM16

/**
  * @brief Reset and Clock Control
  */

typedef struct
{
   __IO uint32_t CR1;   /* Control register 1       0x00 */
   __IO uint32_t CR2;   /* Control register 2       0x04 */
   __IO uint32_t SMCR;  /* Slave mode control reg   0x08 */
   __IO uint32_t DIER;  /* DMA interrupt enable     0x0C */
   __IO uint32_t SR;    /* Status Register          0x10 */
   __IO uint32_t EGR;   /* Event Generation reg     0x14 */
   __IO uint32_t CCMR1; /* CCMR1                    0x18 */
   uint32_t reserved;   /*                          0x1C */
   __IO uint32_t CCER;  /* Capture compare en reg   0x20 */
   __IO uint32_t CNT;   /* Counter                  0x24 */
   __IO uint32_t PSC;   /* Prescaler                0x28 */
   __IO uint32_t ARR;   /* Auto-reload reg          0x2C */
   __IO uint32_t RCR;   /* Repitition Counter Reg   0x30 */
   __IO uint32_t CCR1;  /* Capture Compare Reg      0x34 */
   __IO uint32_t CCR2;  /* Caputre Compare Reg 2    0x38 */
  uint32_t reserved0;    /*                         0x40 */
   __IO uint32_t BDTR;  /* Break dead time reg      0x44 */
   __IO uint32_t DCR;   /* DMA Control Reg          0x48 */
   __IO uint32_t DMAR;  /* DMA addr full transfer   0x4c */
   __IO uint32_t OR1;   /* Option Reg 1             0x50 */
   uint32_t reserved1;   /* Option Reg 1            0x54 */
   uint32_t reserved2;   /* Option Reg 1            0x58 */
   uint32_t reserved3;   /* Option Reg 1            0x5C */
   __IO uint32_t OR2;   /* Option Reg 2             0x60 */

} TIM_TypeDef;


#define TIM15 ((TIM_TypeDef *) TIM15_BASE)
#define TIM16 ((TIM_TypeDef *) TIM16_BASE)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initializeTIM15PWM(void);
void initializeTIM15Counter(void);
void initializeTIM16Counter(void);
void setTIM15FREQ(int freqHz);
void setTIM16Count(int ms);


#endif