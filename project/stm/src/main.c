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

// Multi-frequency synthesis
#define SYNTHESIS_RATE 100000  // 100 kHz update rate
#define NUM_TEST_FREQ 2        // Testing with 2 frequencies

typedef struct {
    float frequency;
    float phase;        // Phase accumulator (0.0 to 1.0)
    bool is_active;
} FrequencyOscillator;

FrequencyOscillator oscillators[NUM_TEST_FREQ];

///////////////////////////////////////////////////////////////////////////////
// Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

// TIM15 interrupt - Multi-frequency synthesis
void TIM1_BRK_TIM15_IRQHandler(void) {
    if (TIM15->SR & (1 << 0)) {  // UIF - Update Interrupt Flag
        TIM15->SR &= ~(1 << 0);   // Clear flag

        bool output_state = false;

        // Update all oscillators and OR their outputs
        for (int i = 0; i < NUM_TEST_FREQ; i++) {
            if (oscillators[i].is_active) {
                // Increment phase accumulator
                oscillators[i].phase += oscillators[i].frequency / SYNTHESIS_RATE;

                // Wrap phase (0.0 to 1.0)
                if (oscillators[i].phase >= 1.0f) {
                    oscillators[i].phase -= 1.0f;
                }

                // Check if this oscillator is in high state (50% duty cycle)
                if (oscillators[i].phase < 0.5f) {
                    output_state = true;  // OR together
                }
            }
        }

        // Update GPIO
        digitalWrite(SQUARE_OUT_PIN, output_state ? GPIO_HIGH : GPIO_LOW);
    }
}

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
    // Match Lab 4 initialization
    initSystem();

    printf("Multi-frequency synthesis test: 10 Hz + 30 Hz\n");

    // Initialize oscillator 0: 10 Hz
    oscillators[0].frequency = 10.0f;
    oscillators[0].phase = 0.0f;
    oscillators[0].is_active = true;

    // Initialize oscillator 1: 30 Hz
    oscillators[1].frequency = 30.0f;
    oscillators[1].phase = 0.0f;
    oscillators[1].is_active = true;

    printf("Oscillator 0: %.1f Hz\n", oscillators[0].frequency);
    printf("Oscillator 1: %.1f Hz\n", oscillators[1].frequency);
    printf("Synthesis rate: %d Hz\n", SYNTHESIS_RATE);

    // Start TIM15 at 100 kHz for synthesis
    setupSynthesisTimer();

    printf("Synthesis started - PA6 outputs OR of both frequencies\n");

    // Main loop - just wait, interrupt handles everything
    while (1) {
        // Could update frequencies here later when FFT is added
    }

    return 0;
}

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
