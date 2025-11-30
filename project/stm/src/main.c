// main.c
// Musical Tesla Coil - STM32 FFT Processing (Simplified)
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Define NVIC for interrupt control
typedef struct {
    volatile uint32_t ISER[8];
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// Define DMA1 ISR/IFCR for interrupt handling  
typedef struct {
    volatile uint32_t ISR;
    volatile uint32_t IFCR;
} DMA_IRQ_Type;
#define DMA1 ((DMA_IRQ_Type *) 0x40020000UL)


// Project Headers
#include "../lib/STM32L432KC_RCC.h"
#include "../lib/STM32L432KC_GPIO.h"
#include "../lib/STM32L432KC_TIM.h"
#include "../lib/STM32L432KC_FLASH.h"
#include "../lib/STM32L432KC_ADC.h"
#include "../lib/STM32L432KC_DMA.h"
#include "../lib/fft_processing.h"

///////////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////////

#define AUDIO_INPUT_PIN     0       // PA0 - ADC Channel 5
#define ADC_CHANNEL         5       // ADC1_IN5 on PA0
#define SQUARE_OUT_PIN      6       // PA6 - Square wave output to FPGA
#define BUFFER_SIZE         256     // Match FFT_SIZE

///////////////////////////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////////////////////////

uint16_t adc_buffer[BUFFER_SIZE];
volatile bool buffer_ready = false;

FrequencyPeak top_frequencies[NUM_FREQUENCIES];

///////////////////////////////////////////////////////////////////////////////
// Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & (1 << 1)) {  // TCIF1 - Transfer Complete
        DMA1->IFCR |= (1 << 1);
        buffer_ready = true;
    }

    if (DMA1->ISR & (1 << 2)) {  // HTIF1 - Half Transfer
        DMA1->IFCR |= (1 << 2);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

void initSystem(void) {
    configureFlash();
    configureClock();  // 80 MHz PLL

    RCC->AHB2ENR |= (1 << 0);  // GPIOA

    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
    pinMode(SQUARE_OUT_PIN, GPIO_OUTPUT);
    digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
}

void setupTimerForSquareWave(void) {
    // Enable Timer 16 clock
    RCC->APB2ENR |= (1 << 17);  // TIM16EN

    // Configure PA6 for alternate function (TIM16_CH1)
    pinMode(SQUARE_OUT_PIN, GPIO_ALT);
    GPIOA->AFRL &= ~(0xF << (4 * SQUARE_OUT_PIN));
    GPIOA->AFRL |= (0xE << (4 * SQUARE_OUT_PIN));  // AF14 for TIM16

    initTIM16PWM();
}

void updateSquareWaveFrequency(float frequency) {
    if (frequency > 50.0f && frequency < 10000.0f) {
        setTIM16FREQ((uint32_t)frequency);
    } else {
        // Out of range - turn off
        setTIM16FREQ(0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////

int main(void) {
    initSystem();
    initFFT();

    configureADCForDMA(ADC_CHANNEL);
    initDMA_ADC(adc_buffer, BUFFER_SIZE);

    // Enable DMA interrupt
    NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn

    enableDMA_ADC();
    startADC();

    setupTimerForSquareWave();

    printf("Musical Tesla Coil - Simplified FFT Started\n");
    printf("FFT size: %d, Sample rate: %d Hz\n", FFT_SIZE, SAMPLE_RATE);

    while (1) {
        if (buffer_ready) {
            buffer_ready = false;

            // Process FFT
            processFFT(adc_buffer, top_frequencies);

            // Debug output
            printf("Top 5 Frequencies:\n");
            for (int i = 0; i < NUM_FREQUENCIES; i++) {
                printf("  %d: %.1f Hz (mag: %.1f)\n",
                       i + 1,
                       top_frequencies[i].frequency,
                       top_frequencies[i].magnitude);
            }

            // Output to FPGA via timer PWM
            updateSquareWaveFrequency(top_frequencies[0].frequency);
        }
    }

    return 0;
}
