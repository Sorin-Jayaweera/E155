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
   __IO uint16_t CR1;   /* Control register 1       0x00 */
   //insert reserved uint16

   __IO uint16_t CR2;   /* Control register 2       0x04 */
   __IO uint16_t SMCR;  /* Slave mode control reg   0x08 */
   __IO uint16_t DIER;  /* DMA interrupt enable     0x0C */
   __IO uint16_t SR;    /* Status Register          0x10 */
   __IO uint16_t EGR;   /* Event Generation reg     0x14 */
   __IO uint32_t CCMR1; /* CCMR1                    0x18 */
   __IO uint16_t CCER;  /* Capture compare en reg   0x20 */
   __IO uint16_t CNT;   /* Counter                  0x24 */
   __IO uint16_t PSC;   /* Prescaler                0x28 */
   __IO uint16_t ARR;   /* Auto-reload reg          0x2C */
   __IO uint16_t RCR;   /* Repitition Counter Reg   0x30 */
   __IO uint16_t CCR1;  /* Capture Compare Reg      0x34 */
   __IO uint16_t CCR2;  /* Caputre Compare Reg 2    0x38 */
   __IO uint32_t BDTR;  /* Break dead time reg      0x44 */
   __IO uint16_t DCR;   /* DMA Control Reg          0x48 */
   __IO uint16_t DMAR;  /* DMA addr full transfer   0x4c */
   __IO uint32_t OR1;   /* Option Reg 1             0x50 */
   __IO uint32_t OR2;   /* Option Reg 2             0x60 */

} TIM15_TypeDef;

typedef struct
{
   __IO uint16_t CR1;   /* Control register 1          0x00 */
   __IO uint16_t CR2;   /* Control register 2          0x04 */
   __IO uint16_t DIER;  /* DMA interrupt enable        0x0C */
   __IO uint16_t SR;    /* Status register             0x10 */
   __IO uint16_t EGR;   /* Event Generation register   0x14 */
   __IO uint32_t CCMR1; /* Capture Compare Mode Reg 1  0x18 */
   __IO uint16_t CCER;  /* Capture compare enable reg  0x20 */
   __IO uint16_t CNT;   /* Counter                     0x24 */
   __IO uint16_t PSC;   /* Prescaler                   0x28 */
   __IO uint16_t ARR;   /* Auto-Reload Register        0x2C */
   __IO uint16_t RCR;   /* Repitition counter register 0x30 */
   __IO uint16_t CCR1;  /* Capture compare register 1  0x34 */
   __IO uint32_t BDTR;  /* Break & dead-time register  0x44 */
   __IO uint16_t DCR;   /* DMA Control                 0x48 */
   __IO uint16_t DMAR;  /* DMA address for full trans  0x4C */
   __IO uint32_t OR1;   /* option register 1           0x50 */
   __IO uint32_t OR2;   /* option register 2           0x60 */

} TIM16_TypeDef;

#define TIM15 ((TIM15_TypeDef *) TIM15_BASE)
#define TIM16 ((TIM16_TypeDef *) TIM16_BASE)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initializeTIM15PWM(void);
void initializeTIM16Counter(void);

void setTIM15FREQ(int freqHz);
void setTIM16Count(int ms);




#endif