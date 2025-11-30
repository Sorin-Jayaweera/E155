// main.c
// Musical Tesla Coil - STM32 FFT Processing
//
// This program:
// 1. Samples audio input from DFPLAYER mini using ADC
// 2. Performs FFT to extract frequency components
// 3. Identifies top 5 dominant frequencies
// 4. Outputs square wave to FPGA for frequency visualization

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_FLASH.h"
#include "STM32L432KC_ADC.h"
#include "STM32L432KC_DMA.h"
#include "fft_processing.h"

///////////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////////

#define AUDIO_INPUT_PIN     0       // PA0 - ADC Channel 5 (can be changed)
#define ADC_CHANNEL         5       // ADC1_IN5 on PA0
#define SQUARE_OUT_PIN      6       // PA6 - Square wave output to FPGA
#define BUFFER_SIZE         512     // Must match FFT_SIZE

///////////////////////////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////////////////////////

// Double buffer for ADC samples
uint16_t adc_buffer[BUFFER_SIZE];
volatile bool buffer_ready = false;

// Frequency peaks
FrequencyPeak top_frequencies[NUM_FREQUENCIES];

///////////////////////////////////////////////////////////////////////////////
// Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

// DMA1 Channel 1 interrupt handler (for ADC)
void DMA1_Channel1_IRQHandler(void) {
    // Check if transfer complete flag is set
    if (DMA1->ISR & (1 << 1)) {  // TCIF1
        // Clear the flag
        DMA1->IFCR |= (1 << 1);

        // Signal that buffer is ready for processing
        buffer_ready = true;
    }

    // Check if half transfer flag is set (optional - for double buffering)
    if (DMA1->ISR & (1 << 2)) {  // HTIF1
        // Clear the flag
        DMA1->IFCR |= (1 << 2);
        // Could process first half here for true double buffering
    }
}

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

void initSystem(void) {
    // Configure flash and clock
    configureFlash();
    configureClock();  // 80 MHz PLL

    // Enable GPIO clocks
    RCC->AHB2ENR |= (1 << 0);  // GPIOA

    // Configure audio input pin as analog
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);

    // Configure square wave output pin
    pinMode(SQUARE_OUT_PIN, GPIO_OUTPUT);
    digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
}

void outputFrequencyToFPGA(FrequencyPeak* peaks) {
    // Output the dominant frequency as a square wave
    // The FPGA counts edges in a 100ms window

    // Get the most dominant frequency
    float dominant_freq = peaks[0].frequency;

    if (dominant_freq > 0 && peaks[0].magnitude > 100.0f) {  // Threshold to filter noise
        // Generate square wave at the dominant frequency
        // This is a simple approach - toggle at 2x the frequency
        // For more sophisticated encoding, you could use PWM or pulse trains

        // Calculate half period in microseconds
        uint32_t half_period_us = (uint32_t)(500000.0f / dominant_freq);

        // Generate a few pulses (adjust number as needed)
        for (int i = 0; i < 10; i++) {
            digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
            for (volatile uint32_t j = 0; j < half_period_us * 80; j++);  // Delay
            digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
            for (volatile uint32_t j = 0; j < half_period_us * 80; j++);  // Delay
        }
    } else {
        // No significant frequency - keep output low
        digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
    }
}

// Alternative: Use timer to generate precise square wave
void setupTimerForSquareWave(void) {
    // Enable Timer 16 clock
    RCC->APB2ENR |= (1 << 17);  // TIM16EN

    // Configure PA6 for alternate function (TIM16_CH1)
    pinMode(SQUARE_OUT_PIN, GPIO_ALT);
    GPIOA->AFRL &= ~(0xF << (4 * SQUARE_OUT_PIN));
    GPIOA->AFRL |= (0xE << (4 * SQUARE_OUT_PIN));  // AF14 for TIM16

    // Initialize TIM16 for PWM
    initTIM16PWM();
}

void updateSquareWaveFrequency(float frequency) {
    if (frequency > 0) {
        // Set timer frequency based on dominant frequency
        setTIM16FREQ((uint32_t)frequency);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////

int main(void) {
    // Initialize system
    initSystem();

    // Initialize FFT
    initFFT();

    // Initialize ADC for DMA
    configureADCForDMA(ADC_CHANNEL);

    // Initialize DMA for ADC
    initDMA_ADC(adc_buffer, BUFFER_SIZE);

    // Enable DMA interrupt
    NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn = 11

    // Enable DMA and ADC
    enableDMA_ADC();
    startADC();

    // Optional: Set up timer for square wave output
    setupTimerForSquareWave();

    printf("Musical Tesla Coil - FFT Processing Started\n");
    printf("Sampling at %d Hz, FFT size: %d\n", SAMPLE_RATE, FFT_SIZE);

    // Main loop
    while (1) {
        // Wait for buffer to be ready
        if (buffer_ready) {
            buffer_ready = false;

            // Process FFT
            processFFT(adc_buffer, top_frequencies);

            // Output results (for debugging)
            printf("Top 5 Frequencies:\n");
            for (int i = 0; i < NUM_FREQUENCIES; i++) {
                printf("  %d: %.2f Hz (mag: %.2f)\n",
                       i + 1,
                       top_frequencies[i].frequency,
                       top_frequencies[i].magnitude);
            }

            // Send frequency to FPGA
            // Method 1: Direct square wave generation
            // outputFrequencyToFPGA(top_frequencies);

            // Method 2: Use timer PWM (more accurate)
            updateSquareWaveFrequency(top_frequencies[0].frequency);
        }

        // Optional: Add delay or sleep mode to reduce power consumption
    }

    return 0;
}
