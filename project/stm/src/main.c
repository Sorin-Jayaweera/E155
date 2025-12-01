// main.c
// Musical Tesla Coil - STM32 FFT Processing with Multi-Frequency Synthesis

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Define NVIC for interrupt control (not in library headers)
typedef struct {
    volatile uint32_t ISER[8];
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// Project Headers - these define TIM15, TIM_TypeDef, DMA1, DMA_TypeDef
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

// FFT-based frequency detection
uint16_t adc_buffer[BUFFER_SIZE];
volatile bool buffer_ready = false;
FrequencyPeak top_frequencies[NUM_FREQUENCIES];

// ============================================================================
// WORKING: Multi-frequency synthesis (10 Hz + 30 Hz OR-ed together)
// ============================================================================
// #define SYNTHESIS_RATE 100000  // 100 kHz update rate
// #define NUM_TEST_FREQ 2        // Testing with 2 frequencies
//
// typedef struct {
//     float frequency;
//     float phase;        // Phase accumulator (0.0 to 1.0)
//     bool is_active;
// } FrequencyOscillator;
//
// FrequencyOscillator oscillators[NUM_TEST_FREQ];
// ============================================================================

///////////////////////////////////////////////////////////////////////////////
// Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

// DMA1 Channel 1 interrupt handler (for ADC)
void DMA1_Channel1_IRQHandler(void) {
    // Check if transfer complete flag is set
    if (DMA1->ISR & (1 << 1)) {  // TCIF1
        DMA1->IFCR |= (1 << 1);  // Clear flag
        buffer_ready = true;      // Signal main loop
    }

    // Half transfer flag (optional for double buffering)
    if (DMA1->ISR & (1 << 2)) {  // HTIF1
        DMA1->IFCR |= (1 << 2);
    }
}

// ============================================================================
// WORKING: Multi-frequency synthesis interrupt handler
// ============================================================================
// void TIM1_BRK_TIM15_IRQHandler(void) {
//     if (TIM15->SR & (1 << 0)) {
//         TIM15->SR &= ~(1 << 0);
//         bool output_state = false;
//         for (int i = 0; i < NUM_TEST_FREQ; i++) {
//             if (oscillators[i].is_active) {
//                 oscillators[i].phase += oscillators[i].frequency / SYNTHESIS_RATE;
//                 if (oscillators[i].phase >= 1.0f) {
//                     oscillators[i].phase -= 1.0f;
//                 }
//                 if (oscillators[i].phase < 0.5f) {
//                     output_state = true;
//                 }
//             }
//         }
//         digitalWrite(SQUARE_OUT_PIN, output_state ? GPIO_HIGH : GPIO_LOW);
//     }
// }
// ============================================================================

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

void initSystem(void) {
    // Match Lab 4 initialization sequence
    configureFlash();
    configureClock();  // 80 MHz PLL

    // Enable GPIOA clock
    RCC->AHB2ENR |= (1 << 0);  // GPIO A

    // Configure pins
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
    pinMode(SQUARE_OUT_PIN, GPIO_OUTPUT);
    digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);

    printf("System initialized\n");
    printf("RCC APB2ENR: %lu\n", RCC->APB2ENR);
    printf("RCC CFGR: %lu\n", RCC->CFGR);
    printf("RCC AHB2ENR: %lu\n", RCC->AHB2ENR);
}

void setupSynthesisTimer(void) {
    // Enable TIM15
    RCC->APB2ENR |= (1 << 16);  // TIM15EN
    
    // Configure for 100 kHz interrupt
    TIM15->PSC = 0;                        // No prescaler (80 MHz)
    TIM15->ARR = 800 - 1;                  // 80 MHz / 800 = 100 kHz
    
    // Enable update interrupt
    TIM15->DIER |= (1 << 0);  // UIE
    
    // Enable TIM15 interrupt in NVIC
    NVIC->ISER[0] |= (1 << 24);  // TIM1_BRK_TIM15_IRQn
    
    // Start timer
    TIM15->CR1 |= (1 << 0);  // CEN
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////

int main(void) {
    // Initialize system
    initSystem();
    initFFT();

    printf("\n===== FFT-Based Frequency Detection =====\n");
    printf("FFT size: %d\n", FFT_SIZE);
    printf("Sample rate: %d Hz\n", SAMPLE_RATE);
    printf("Frequency resolution: %.2f Hz per bin\n", (float)SAMPLE_RATE / FFT_SIZE);
    printf("Feed 50 Hz signal to PA0\n\n");

    // Configure ADC for DMA
    configureADCForDMA(ADC_CHANNEL);
    initDMA_ADC(adc_buffer, BUFFER_SIZE);

    // Enable DMA interrupt
    NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn = 11

    // Start ADC with DMA
    enableDMA_ADC();
    startADC();

    printf("ADC started, waiting for samples...\n\n");

    // Main loop - process FFT when buffer is ready
    while (1) {
        if (buffer_ready) {
            buffer_ready = false;

            // Process FFT to find dominant frequencies
            processFFT(adc_buffer, top_frequencies);

            // Print top 5 frequencies
            printf("Top 5 Frequencies:\n");
            for (int i = 0; i < NUM_FREQUENCIES; i++) {
                printf("  %d: %.1f Hz (magnitude: %.1f)\n",
                       i + 1,
                       top_frequencies[i].frequency,
                       top_frequencies[i].magnitude);
            }
            printf("\n");
        }
    }

    return 0;
}

// ============================================================================
// WORKING: Multi-frequency synthesis main code
// ============================================================================
// int main(void) {
//     initSystem();
//     printf("Multi-frequency synthesis test: 10 Hz + 30 Hz\n");
//     oscillators[0].frequency = 10.0f;
//     oscillators[0].phase = 0.0f;
//     oscillators[0].is_active = true;
//     oscillators[1].frequency = 30.0f;
//     oscillators[1].phase = 0.0f;
//     oscillators[1].is_active = true;
//     printf("Oscillator 0: %.1f Hz\n", oscillators[0].frequency);
//     printf("Oscillator 1: %.1f Hz\n", oscillators[1].frequency);
//     printf("Synthesis rate: %d Hz\n", SYNTHESIS_RATE);
//     setupSynthesisTimer();
//     printf("Synthesis started - PA6 outputs OR of both frequencies\n");
//     while (1) { }
//     return 0;
// }
// ============================================================================

// Commented out for simple test
// int main(void) {
//     initSystem();
//     initFFT();
//     for (int i = 0; i < 10; i++) {
//         digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
//         for (volatile int j = 0; j < 800000; j++);
//         digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
//         for (volatile int j = 0; j < 800000; j++);
//     }
//     for (int i = 0; i < NUM_FREQUENCIES; i++) {
//         oscillators[i].phase = 0.0f;
//         oscillators[i].frequency = 0.0f;
//         oscillators[i].magnitude = 0.0f;
//         oscillators[i].is_active = false;
//     }
//     oscillators[0].frequency = 100.0f;
//     oscillators[0].magnitude = 200.0f;
//     oscillators[0].is_active = true;
//     setupSynthesisTimer();
//     configureADCForDMA(ADC_CHANNEL);
//     initDMA_ADC(adc_buffer, BUFFER_SIZE);
//     NVIC->ISER[0] |= (1 << 11);
//     enableDMA_ADC();
//     startADC();
//     printf("Musical Tesla Coil - Multi-Frequency Synthesis\n");
//     printf("FFT size: %d, Sample rate: %d Hz\n", FFT_SIZE, SAMPLE_RATE);
//     printf("Synthesis rate: %d Hz\n", SYNTHESIS_RATE);
//     while (1) {
//         if (buffer_ready) {
//             buffer_ready = false;
//             processFFT(adc_buffer, top_frequencies);
//             for (int i = 0; i < NUM_FREQUENCIES; i++) {
//                 oscillators[i].frequency = top_frequencies[i].frequency;
//                 oscillators[i].magnitude = top_frequencies[i].magnitude;
//                 oscillators[i].is_active = (top_frequencies[i].magnitude > 10.0f);
//             }
//             printf("Top 5 Frequencies:\n");
//             for (int i = 0; i < NUM_FREQUENCIES; i++) {
//                 printf("  %d: %.1f Hz (mag: %.1f) %s\n",
//                        i + 1,
//                        top_frequencies[i].frequency,
//                        top_frequencies[i].magnitude,
//                        oscillators[i].is_active ? "[ACTIVE]" : "");
//             }
//         }
//     }
//     return 0;
// }
