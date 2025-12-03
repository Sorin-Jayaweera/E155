// STM32L432KC_ADC.h
// Header for ADC functions

#ifndef STM32L4_ADC_H
#define STM32L4_ADC_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS device header (provides ADC_TypeDef, ADC1, etc.)

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
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initADC(uint8_t channel);
void configureADCForDMA(uint8_t channel);
void startADC(void);
void stopADC(void);
uint16_t readADC(void);
void calibrateADC(void);

#endif
