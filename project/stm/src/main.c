// main.c
// Musical Tesla Coil - Frequency Detection with LED Feedback
// LED turns ON when input frequency is 400-600 Hz (500 Hz ± 100 Hz)

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
#define TEST_INPUT_PIN      7       // PB7 - Test input pin
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

    // Initialize ADC with DMA
    configureADCForDMA(ADC_CHANNEL);
    initDMA_ADC(adc_buffer, BUFFER_SIZE);

    // Enable DMA1 Channel 1 interrupt in NVIC
    NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn

    // Enable DMA and start ADC
    enableDMA_ADC();
    startADC();

    // Target frequency: 500 Hz ± 100 Hz (400-600 Hz range)
    const float TARGET_FREQ_MIN = 400.0f;
    const float TARGET_FREQ_MAX = 600.0f;

    // Main loop - LED-based frequency detection
    while (1) {
        // Wait for DMA buffer to be ready
        if (buffer_ready) {
            buffer_ready = false;

            // Process FFT to get top frequencies
            processFFT(adc_buffer, top_frequencies);

            // Check if dominant frequency (top_frequencies[0]) is in target range
            float dominant_freq = top_frequencies[0].frequency;

            if (dominant_freq >= TARGET_FREQ_MIN && dominant_freq <= TARGET_FREQ_MAX) {
                // Frequency is in range (400-600 Hz) - turn LED ON
                digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
            } else {
                // Frequency is out of range - turn LED OFF
                digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
            }
        }
    }

    return 0;
}

// ============================================================================
// UART Communication Test (commented out)
// ============================================================================
// int main(void) {
//     initSystem();
//     printf("\n");
//     printf("========================================\n");
//     printf("  UART Communication Test\n");
//     printf("========================================\n");
//     printf("If you can see this, UART is working!\n");
//     printf("System clock: 80 MHz\n");
//     printf("Baud rate: 115200\n");
//     printf("\n");
//     printf("Printing heartbeat messages every second...\n");
//     printf("\n");
//     int counter = 0;
//     while (1) {
//         printf("Heartbeat %d - UART is alive!\n", counter);
//         digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
//         for (volatile int i = 0; i < 4000000; i++);  // ~0.5s
//         digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
//         for (volatile int i = 0; i < 4000000; i++);  // ~0.5s
//         counter++;
//     }
//     return 0;
// }
// ============================================================================

// ============================================================================
// ADC Input Test (commented out)
// ============================================================================
// int main(void) {
//     initSystem();
//     printf("\n===== ADC Input Test =====\n");
//     printf("Reading analog signal from PA0 (ADC1_IN5)\n");
//     printf("Connect sine wave (0-3.3V) to PA0\n");
//     printf("Sampling continuously and printing values...\n\n");
//     RCC->AHB2ENR |= (1 << 13);
//     pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
//     configureADCForDMA(ADC_CHANNEL);
//     startADC();
//     printf("ADC initialized, starting continuous sampling...\n\n");
//     int sample_count = 0;
//     while (1) {
//         ADC1->CR |= (1 << 2);
//         while (!(ADC1->ISR & (1 << 2)));
//         uint16_t adc_value = ADC1->DR;
//         float voltage = (adc_value * 3.3f) / 4095.0f;
//         if (sample_count % 100 == 0) {
//             printf("Sample %d: ADC=%u (%.3f V)\n", sample_count, adc_value, voltage);
//         }
//         sample_count++;
//         for (volatile int i = 0; i < 8000; i++);
//     }
//     return 0;
// }
// ============================================================================

// ============================================================================
// GPIO Input Test (PB7 → PA6 passthrough - WORKING)
// ============================================================================
// int main(void) {
//     initSystem();
//     printf("\n===== GPIO Input → LED Test =====\n");
//     printf("PB7 (input) → PA6 (LED output)\n");
//     printf("Connect signal to PB7, LED mirrors input state\n\n");
//     RCC->AHB2ENR |= (1 << 1);  // GPIOB
//     GPIOB->MODER &= ~(0b11 << (2 * TEST_INPUT_PIN));
//     printf("PB7 configured as input, starting passthrough...\n\n");
//     while (1) {
//         int input_state = (GPIOB->IDR >> TEST_INPUT_PIN) & 1;
//         digitalWrite(SQUARE_OUT_PIN, input_state);
//     }
//     return 0;
// }
// ============================================================================

// ============================================================================
// FFT-based frequency detection (commented out for GPIO test)
// ============================================================================
// int main(void) {
//     initSystem();
//     initFFT();
//     printf("\n===== FFT-Based Frequency Detection =====\n");
//     printf("FFT size: %d\n", FFT_SIZE);
//     printf("Sample rate: %d Hz\n", SAMPLE_RATE);
//     printf("Frequency resolution: %.2f Hz per bin\n", (float)SAMPLE_RATE / FFT_SIZE);
//     printf("Feed 50 Hz signal to PA0\n\n");
//     configureADCForDMA(ADC_CHANNEL);
//     initDMA_ADC(adc_buffer, BUFFER_SIZE);
//     NVIC->ISER[0] |= (1 << 11);
//     enableDMA_ADC();
//     startADC();
//     printf("ADC started, waiting for samples...\n\n");
//     while (1) {
//         if (buffer_ready) {
//             buffer_ready = false;
//             processFFT(adc_buffer, top_frequencies);
//             printf("Top 5 Frequencies:\n");
//             for (int i = 0; i < NUM_FREQUENCIES; i++) {
//                 printf("  %d: %.1f Hz (magnitude: %.1f)\n",
//                        i + 1,
//                        top_frequencies[i].frequency,
//                        top_frequencies[i].magnitude);
//             }
//             printf("\n");
//         }
//     }
//     return 0;
// }
// ============================================================================

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
