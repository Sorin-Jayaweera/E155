// STM32L432KC_RCC.h
// Header for RCC functions

#ifndef STM32L4_TIM_H
#define STM32L4_TIM_H

#include <stdint.h>

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
   __IO uint16_t CR1;    /*!<  0x40 */
   __IO uint16_t CR2; 
   __IO uint16_t SMCR;
   __IO uint16_t DIER;
   __IO uint16_t SR;
   __IO uint16_t EGR;
   __IO uint32_t CCMR1;
   __IO uint16_t CCER;
   __IO uint16_t CNT;
   __IO uint16_t PSC;
   __IO uint16_t ARR;
   __IO uint16_t RCR;
   __IO uint16_t CCR1;
   __IO uint16_t CCR2;   
   __IO uint32_t BDTR;
   __IO uint16_t DCR;
   __IO uint16_t DMAR;
   __IO uint32_t OR1;
   __IO uint32_t OR2;  

} TIM15_TypeDef;

typedef struct
{
   __IO uint16_t CR1;   /* Control register 1          0x00*/
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

void configureTIM(void);
void configurePWM(void);
void configureCOUNTER(void);


#endif