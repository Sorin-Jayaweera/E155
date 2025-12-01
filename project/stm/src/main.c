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

// Simple 100 Hz test - toggle every 500 interrupts (100kHz / 500 = 200Hz toggles = 100Hz square wave)
volatile uint16_t toggle_counter = 0;
#define TOGGLE_PERIOD 500  // 100kHz / 500 = 200 toggles/sec = 100Hz

// Commented out for simple test
// #define SYNTHESIS_RATE 100000  // 100 kHz update rate
// typedef struct {
//     float frequency;
//     float phase;
//     float magnitude;
//     bool is_active;
// } FrequencyOscillator;
// FrequencyOscillator oscillators[NUM_FREQUENCIES];
// uint16_t adc_buffer[BUFFER_SIZE];
// volatile bool buffer_ready = false;
// FrequencyPeak top_frequencies[NUM_FREQUENCIES];

///////////////////////////////////////////////////////////////////////////////
// Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

// Simple TIM15 interrupt - just toggle PA6 at 100 Hz
void TIM1_BRK_TIM15_IRQHandler(void) {
    if (TIM15->SR & (1 << 0)) {  // UIF - Update Interrupt Flag
        TIM15->SR &= ~(1 << 0);   // Clear flag

        toggle_counter++;
        if (toggle_counter >= TOGGLE_PERIOD) {
            toggle_counter = 0;
            // Toggle PA6
            GPIOA->ODR ^= (1 << SQUARE_OUT_PIN);
        }
    }
}

// Commented out for simple test
// void DMA1_Channel1_IRQHandler(void) {
//     if (DMA1->ISR & (1 << 1)) {
//         DMA1->IFCR |= (1 << 1);
//         buffer_ready = true;
//     }
//     if (DMA1->ISR & (1 << 2)) {
//         DMA1->IFCR |= (1 << 2);
//     }
// }

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

    printf("Setting PA6 HIGH permanently...\n");
    printf("Check with multimeter - should read ~3.3V\n");

    // Set PA6 HIGH and leave it there
    digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);

    printf("PA6 is now HIGH\n");
    printf("Voltage should be stable at 3.3V\n");

    // Just loop forever, keep it HIGH
    while (1) {
        // Do nothing - PA6 stays HIGH
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
