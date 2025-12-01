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

typedef struct {
    float frequency;
    float phase;        // Phase accumulator (0.0 to 1.0)
    float magnitude;
    bool is_active;
} FrequencyOscillator;

FrequencyOscillator oscillators[NUM_FREQUENCIES];

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

void TIM1_BRK_TIM15_IRQHandler(void) {
    if (TIM15->SR & (1 << 0)) {  // UIF
        TIM15->SR &= ~(1 << 0);   // Clear flag
        
        bool output_state = false;
        
        // Update all oscillators
        for (int i = 0; i < NUM_FREQUENCIES; i++) {
            if (oscillators[i].is_active) {
                // Increment phase
                oscillators[i].phase += oscillators[i].frequency / SYNTHESIS_RATE;
                
                // Wrap phase
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
    configureFlash();
    configureClock();  // 80 MHz PLL

    RCC->AHB2ENR |= (1 << 0);  // GPIOA

    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
    pinMode(SQUARE_OUT_PIN, GPIO_OUTPUT);
    digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
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
//    configureFlash();
//    configureClock();
    initSystem();
    initFFT();
    
    printf("Musical Tesla Coil - Multi-Frequency Synthesis\n");
    // Initialize oscillators
    for (int i = 0; i < NUM_FREQUENCIES; i++) {
        oscillators[i].phase = 0.0f;
        oscillators[i].frequency = 0.0f;
        oscillators[i].magnitude = 0.0f;
        oscillators[i].is_active = false;
    }
    
    setupSynthesisTimer();  // Multi-frequency output via TIM15 interrupt
    
    configureADCForDMA(ADC_CHANNEL);
    initDMA_ADC(adc_buffer, BUFFER_SIZE);

    // Enable DMA interrupt
    NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn

    enableDMA_ADC();
    startADC();

    printf("Musical Tesla Coil - Multi-Frequency Synthesis\n");
    printf("FFT size: %d, Sample rate: %d Hz\n", FFT_SIZE, SAMPLE_RATE);
    printf("Synthesis rate: %d Hz\n", SYNTHESIS_RATE);

    while (1) {
        if (buffer_ready) {
            buffer_ready = false;

            // Process FFT
            processFFT(adc_buffer, top_frequencies);
            
            // Update oscillators with new frequencies
            for (int i = 0; i < NUM_FREQUENCIES; i++) {
                oscillators[i].frequency = top_frequencies[i].frequency;
                oscillators[i].magnitude = top_frequencies[i].magnitude;
                oscillators[i].is_active = (top_frequencies[i].magnitude > 100.0f);
                // Keep existing phase for smooth transitions
            }

            // Debug output
            printf("Top 5 Frequencies:\n");
            for (int i = 0; i < NUM_FREQUENCIES; i++) {
                printf("  %d: %.1f Hz (mag: %.1f) %s\n",
                       i + 1,
                       top_frequencies[i].frequency,
                       top_frequencies[i].magnitude,
                       oscillators[i].is_active ? "[ACTIVE]" : "");
            }
        }
    }

    return 0;
}
