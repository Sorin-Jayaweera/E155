// STM32L432KC_ADC.h
// Header for ADC functions

#ifndef STM32L4_ADC_H
#define STM32L4_ADC_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// ADC Channels
#define ADC_CHANNEL_0   0
#define ADC_CHANNEL_1   1
#define ADC_CHANNEL_2   2
#define ADC_CHANNEL_3   3
#define ADC_CHANNEL_4   4
#define ADC_CHANNEL_5   5
#define ADC_CHANNEL_6   6
#define ADC_CHANNEL_7   7
#define ADC_CHANNEL_8   8
#define ADC_CHANNEL_9   9
#define ADC_CHANNEL_10  10

// ADC Clock modes (CKMODE in ADC_CCR)
#define ADC_CLOCK_ASYNC_DIV1    0b00
#define ADC_CLOCK_ASYNC_DIV2    0b01
#define ADC_CLOCK_ASYNC_DIV4    0b10
#define ADC_CLOCK_ASYNC_DIV6    0b11
#define ADC_CLOCK_SYNC_DIV1     0b01
#define ADC_CLOCK_SYNC_DIV2     0b10
#define ADC_CLOCK_SYNC_DIV4     0b11

// ADC Resolution
#define ADC_RES_12BIT   0b00
#define ADC_RES_10BIT   0b01
#define ADC_RES_8BIT    0b10
#define ADC_RES_6BIT    0b11

// ADC Sampling time
#define ADC_SMPTIME_2_5     0b000
#define ADC_SMPTIME_6_5     0b001
#define ADC_SMPTIME_12_5    0b010
#define ADC_SMPTIME_24_5    0b011
#define ADC_SMPTIME_47_5    0b100
#define ADC_SMPTIME_92_5    0b101
#define ADC_SMPTIME_247_5   0b110
#define ADC_SMPTIME_640_5   0b111

///////////////////////////////////////////////////////////////////////////////
// ADC register structures
///////////////////////////////////////////////////////////////////////////////

#ifndef __STM32L432xx_H
// Only define these if standard CMSIS library (stm32l432xx.h) is NOT included

#define __IO volatile

// Base addresses
#define ADC1_BASE       (0x50040000UL)
#define ADC_COMMON_BASE (0x50040300UL)

typedef struct {
    __IO uint32_t ISR;      // ADC interrupt and status register,          offset: 0x00
    __IO uint32_t IER;      // ADC interrupt enable register,               offset: 0x04
    __IO uint32_t CR;       // ADC control register,                        offset: 0x08
    __IO uint32_t CFGR;     // ADC configuration register,                  offset: 0x0C
    __IO uint32_t CFGR2;    // ADC configuration register 2,                offset: 0x10
    __IO uint32_t SMPR1;    // ADC sample time register 1,                  offset: 0x14
    __IO uint32_t SMPR2;    // ADC sample time register 2,                  offset: 0x18
    uint32_t RESERVED1;     // Reserved,                                    offset: 0x1C
    __IO uint32_t TR1;      // ADC watchdog threshold register 1,           offset: 0x20
    __IO uint32_t TR2;      // ADC watchdog threshold register 2,           offset: 0x24
    __IO uint32_t TR3;      // ADC watchdog threshold register 3,           offset: 0x28
    uint32_t RESERVED2;     // Reserved,                                    offset: 0x2C
    __IO uint32_t SQR1;     // ADC regular sequence register 1,             offset: 0x30
    __IO uint32_t SQR2;     // ADC regular sequence register 2,             offset: 0x34
    __IO uint32_t SQR3;     // ADC regular sequence register 3,             offset: 0x38
    __IO uint32_t SQR4;     // ADC regular sequence register 4,             offset: 0x3C
    __IO uint32_t DR;       // ADC regular data register,                   offset: 0x40
    uint32_t RESERVED3;     // Reserved,                                    offset: 0x44
    uint32_t RESERVED4;     // Reserved,                                    offset: 0x48
    __IO uint32_t JSQR;     // ADC injected sequence register,              offset: 0x4C
    uint32_t RESERVED5[4];  // Reserved,                                    offset: 0x50-0x5C
    __IO uint32_t OFR1;     // ADC offset register 1,                       offset: 0x60
    __IO uint32_t OFR2;     // ADC offset register 2,                       offset: 0x64
    __IO uint32_t OFR3;     // ADC offset register 3,                       offset: 0x68
    __IO uint32_t OFR4;     // ADC offset register 4,                       offset: 0x6C
    uint32_t RESERVED6[4];  // Reserved,                                    offset: 0x70-0x7C
    __IO uint32_t JDR1;     // ADC injected data register 1,                offset: 0x80
    __IO uint32_t JDR2;     // ADC injected data register 2,                offset: 0x84
    __IO uint32_t JDR3;     // ADC injected data register 3,                offset: 0x88
    __IO uint32_t JDR4;     // ADC injected data register 4,                offset: 0x8C
    uint32_t RESERVED7[4];  // Reserved,                                    offset: 0x90-0x9C
    __IO uint32_t AWD2CR;   // ADC analog watchdog 2 configuration register,offset: 0xA0
    __IO uint32_t AWD3CR;   // ADC analog watchdog 3 configuration register,offset: 0xA4
    uint32_t RESERVED8;     // Reserved,                                    offset: 0xA8
    uint32_t RESERVED9;     // Reserved,                                    offset: 0xAC
    __IO uint32_t DIFSEL;   // ADC differential mode selection register,    offset: 0xB0
    __IO uint32_t CALFACT;  // ADC calibration factors,                     offset: 0xB4
} ADC_TypeDef;

typedef struct {
    __IO uint32_t CSR;      // ADC common status register,                  offset: 0x00
    uint32_t RESERVED;      // Reserved,                                    offset: 0x04
    __IO uint32_t CCR;      // ADC common control register,                 offset: 0x08
    __IO uint32_t CDR;      // ADC common regular data register,            offset: 0x0C
} ADC_Common_TypeDef;

#define ADC1        ((ADC_TypeDef *) ADC1_BASE)
#define ADC_COMMON  ((ADC_Common_TypeDef *) ADC_COMMON_BASE)

#endif // __STM32L432xx_H

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initADC(uint8_t channel);
void configureADCForDMA(uint8_t channel);
void startADC(void);
void stopADC(void);
uint16_t readADC(void);
void calibrateADC(void);

#endif
