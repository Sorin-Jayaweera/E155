/*******************************************************************************
 * main.c - STM32L432KC Musical Tesla Coil with DFPlayer MP3 Control
 *
 * PURPOSE:
 *   Combined system that:
 *   1. Plays music via DFPlayer Mini MP3 module
 *   2. Analyzes audio via FFT to detect dominant frequencies
 *   3. Drives Tesla coil with frequency-matched square waves
 *   4. Allows button control of music playback
 *
 * HARDWARE CONFIGURATION:
 *   FFT System:
 *     - Input:  PA5 (Board A4, ADC Channel 10) - Audio input for FFT
 *     - Output: PB0 (Board D3) - Square wave to Tesla coil
 *
 *   DFPlayer System:
 *     - USART1 TX: PA9 (Board D1) - To DFPlayer RX
 *     - USART1 RX: PA10 (Board D0) - From DFPlayer TX
 *     - Buttons: PA8 (Previous), PA6 (Pause), PB7 (Next)
 *
 * RESOURCE ALLOCATION:
 *   - ADC1: Audio sampling at 8 kHz (PA5, Channel 10)
 *   - USART1: DFPlayer communication at 9600 baud (PA9/PA10)
 *   - TIM2: DFPlayer delay functions
 *   - TIM6: ADC trigger at 8 kHz
 *   - TIM15: FFT synthesis at 100 kHz
 *   - DMA1 Ch1: ADC data transfer
 *
 * AUTHOR: Musical Tesla Coil Integration Project
 * DATE: 2024
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

// Project Libraries (DFPlayer CMSIS-compatible base)
#include "STM32L432KC_RCC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_ADC.h"
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_DMA.h"
#include "STM32L432KC_FLASH.h"
#include "STM32L432KC_USART.h"
#include "DFPLAYER_MINI.h"

/*******************************************************************************
 * CONFIGURATION PARAMETERS
 ******************************************************************************/

// FFT Pin Definitions
#define LED_PIN         0       // PB0 (Board D3) - Output square wave to Tesla coil
#define AUDIO_INPUT_PIN PA5     // PA5 (Board A4) - Analog audio input
#define ADC_CHANNEL     10      // ADC1 Channel 10 (maps to PA5)

// FFT Signal Processing Parameters
#define FFT_SIZE        256     // FFT window size (must be power of 2)
#define SAMPLE_RATE     8000    // Sampling frequency in Hz

// FFT Detection Thresholds
#define FREQ_THRESHOLD      100.0f   // Minimum frequency (Hz)
#define MAX_FREQ_THRESHOLD  2000.0f  // Maximum frequency (Hz)
#define MAG_THRESHOLD       10.0f    // Minimum absolute magnitude
#define RELATIVE_MAG_THRESHOLD 0.5f  // Min magnitude relative to strongest

// FFT Synthesis Parameters
#define SYNTHESIS_RATE 100000  // 100 kHz synthesis update rate
#define MAX_OSCILLATORS 5      // Maximum simultaneous frequencies
#define N_FREQ 5               // Number of top frequencies to synthesize

// ADC Trigger Configuration
#define ADC_EXTSEL      13      // TIM6_TRGO

/*******************************************************************************
 * HARDWARE REGISTER DEFINITIONS
 ******************************************************************************/

// Nested Vectored Interrupt Controller (NVIC)
typedef struct {
    volatile uint32_t ISER[8];
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// System Control Block (SCB) - FPU enable
#define SCB_CPACR (*((volatile uint32_t *) 0xE000ED88UL))

// Math Constant
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/*******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

// Complex number for FFT computation
typedef struct {
    float real;
    float imag;
} Complex;

// Frequency oscillator for multi-frequency synthesis
typedef struct {
    float frequency;
    float phase;
    bool is_active;
} FrequencyOscillator;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

// FFT Variables
uint16_t adc_buffer[FFT_SIZE];
volatile bool buffer_ready = false;
Complex fft_buffer[FFT_SIZE];
FrequencyOscillator oscillators[MAX_OSCILLATORS];

// DFPlayer Variable
USART_TypeDef * dfplayer_usart;

/*******************************************************************************
 * FAST FOURIER TRANSFORM IMPLEMENTATION
 ******************************************************************************/

void fft_compute(Complex* data, int n) {
    int i, j;

    // Bit-Reversal Permutation
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j >= bit; bit >>= 1) {
            j -= bit;
        }
        j += bit;
        if (i < j) {
            Complex temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }

    // Cooley-Tukey FFT Butterfly Operations
    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        Complex wlen = {cosf(angle), sinf(angle)};

        for (i = 0; i < n; i += len) {
            Complex w = {1.0f, 0.0f};

            for (j = 0; j < len / 2; j++) {
                Complex u = data[i + j];
                Complex v = {
                    data[i + j + len/2].real * w.real - data[i + j + len/2].imag * w.imag,
                    data[i + j + len/2].real * w.imag + data[i + j + len/2].imag * w.real
                };

                data[i + j].real = u.real + v.real;
                data[i + j].imag = u.imag + v.imag;
                data[i + j + len/2].real = u.real - v.real;
                data[i + j + len/2].imag = u.imag - v.imag;

                float w_temp = w.real;
                w.real = w.real * wlen.real - w.imag * wlen.imag;
                w.imag = w_temp * wlen.imag + w.imag * wlen.real;
            }
        }
    }
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES
 ******************************************************************************/

// DMA1 Channel 1 Transfer Complete Interrupt
void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & (1 << 1)) {
        DMA1->IFCR |= (0xF << 0);
        buffer_ready = true;
    }
}

// TIM15 ISR - Multi-frequency synthesis
void TIM1_BRK_TIM15_IRQHandler(void) {
    if (TIM15->SR & (1 << 0)) {
        TIM15->SR &= ~(1 << 0);

        bool output_state = false;

        for (int i = 0; i < MAX_OSCILLATORS; i++) {
            if (oscillators[i].is_active) {
                oscillators[i].phase += oscillators[i].frequency / SYNTHESIS_RATE;

                if (oscillators[i].phase >= 1.0f) {
                    oscillators[i].phase -= 1.0f;
                }

                if (oscillators[i].phase < 0.5f) {
                    output_state = true;
                }
            }
        }

        // Update GPIO - Direct GPIOB access
        if (output_state) {
            GPIOB->ODR |= (1 << LED_PIN);
        } else {
            GPIOB->ODR &= ~(1 << LED_PIN);
        }
    }
}

/*******************************************************************************
 * HARDWARE INITIALIZATION FUNCTIONS
 ******************************************************************************/

void initSystem(void) {
    // Enable FPU
    SCB_CPACR |= ((3UL << 10*2) | (3UL << 11*2));

    // Enable GPIOA and GPIOB clocks
    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);

    // Configure PB0 as digital output (Direct GPIOB register access)
    GPIOB->MODER |= (0b1 << (2 * LED_PIN));
    GPIOB->MODER &= ~(0b1 << (2 * LED_PIN + 1));

    // Configure PA5 as analog input
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
}

void initADC_DMA(void) {
    // Initialize DMA
    initDMA_ADC(adc_buffer, FFT_SIZE);

    // Enable DMA1 Channel 1 interrupt
    NVIC->ISER[0] |= (1 << 11);

    // Enable ADC clock
    RCC->AHB2ENR |= (1 << 13);

    // Exit deep power-down and enable voltage regulator
    ADC1->CR &= ~(1 << 29);
    ADC1->CR |= (1 << 28);

    // Wait for regulator
    for(volatile int i = 0; i < 2000; i++);

    // Configure ADC clock (CMSIS uses ADC123_COMMON)
    ADC123_COMMON->CCR |= (1 << 16);

    // Configure ADC for DMA and hardware trigger
    ADC1->CFGR |= (1 << 0) | (1 << 1) | (1 << 10);

    // Select external trigger
    ADC1->CFGR &= ~(0xF << 6);
    ADC1->CFGR |= (ADC_EXTSEL << 6);

    // Enable ADC
    ADC1->ISR |= (1 << 0);
    ADC1->CR |= (1 << 0);

    // Wait for ADC ready
    while (!(ADC1->ISR & (1 << 0)));

    // Configure conversion sequence
    ADC1->SQR1 = (ADC_CHANNEL << 6);

    // Set sampling time
    ADC1->SMPR2 |= (2U << 3);

    // Start conversions
    ADC1->CR |= (1 << 2);

    // Enable DMA
    enableDMA_ADC();
}

void initDFPlayer(void) {
    // Enable TIM2 for delays
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(TIM2);

    // Initialize USART1 for DFPlayer
    dfplayer_usart = initUSART(USART1_ID, 9600);

    // Initialize DFPlayer with volume 25
    DF_Init(dfplayer_usart, 25);

    // Small delay
    delay_millis(TIM2, 100);

    // Start playing
    DF_PlayFromStart(dfplayer_usart);
}

/*******************************************************************************
 * MAIN PROGRAM
 ******************************************************************************/

int main(void) {
    // Configure Flash and clocks
    configureFlash();
    configureClock();

    // Initialize all subsystems
    initSystem();        // GPIO, FPU
    initADC_DMA();       // ADC and DMA
    initTimer_ADC();     // TIM6 trigger
    initDFPlayer();      // DFPlayer + TIM2

    printf("\n========================================\n");
    printf("  MUSICAL TESLA COIL - INTEGRATED SYSTEM\n");
    printf("========================================\n");
    printf("FFT Configuration:\n");
    printf("  Sample Rate: %d Hz\n", SAMPLE_RATE);
    printf("  FFT Size: %d samples\n", FFT_SIZE);
    printf("  Frequency Range: %.0f - %.0f Hz\n", FREQ_THRESHOLD, MAX_FREQ_THRESHOLD);
    printf("  Audio Input: PA5 (A4)\n");
    printf("  Tesla Output: PB0 (D3)\n\n");
    printf("DFPlayer Configuration:\n");
    printf("  Previous: PA8 (D9)\n");
    printf("  Pause: PA6 (A5)\n");
    printf("  Next: PB7 (D4)\n");
    printf("========================================\n\n");

    // Initialize oscillators
    for (int i = 0; i < MAX_OSCILLATORS; i++) {
        oscillators[i].frequency = 0.0f;
        oscillators[i].phase = 0.0f;
        oscillators[i].is_active = false;
    }

    printf("Starting TIM15 synthesis engine...\n");
    initTIM15_Synthesis();

    printf("System ready! Processing audio...\n\n");

    // Main loop: FFT processing + button checking
    while(1) {
        // Check DFPlayer buttons
        Check_Key(dfplayer_usart);

        // Process FFT when buffer ready
        if (buffer_ready) {
            buffer_ready = false;

            // Convert ADC samples to complex
            for (int i = 0; i < FFT_SIZE; i++) {
                fft_buffer[i].real = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
                fft_buffer[i].imag = 0.0f;
            }

            // Perform FFT
            fft_compute(fft_buffer, FFT_SIZE);

            // Calculate magnitudes
            float magnitudes[FFT_SIZE / 2];
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                float real = fft_buffer[i].real;
                float imag = fft_buffer[i].imag;
                magnitudes[i] = sqrtf(real * real + imag * imag);
            }

            // Find global maximum
            float global_max_mag = 0.0f;
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                if (magnitudes[i] > global_max_mag) {
                    global_max_mag = magnitudes[i];
                }
            }

            float relative_cutoff = global_max_mag * RELATIVE_MAG_THRESHOLD;

            // Find top N_FREQ frequencies
            int active_count = 0;
            for (int n = 0; n < N_FREQ; n++) {
                float max_mag = 0.0f;
                int max_bin = 0;

                for (int i = 1; i < FFT_SIZE / 2; i++) {
                    if (magnitudes[i] > max_mag) {
                        max_mag = magnitudes[i];
                        max_bin = i;
                    }
                }

                float freq = (float)max_bin * SAMPLE_RATE / FFT_SIZE;

                if (freq > FREQ_THRESHOLD &&
                    freq < MAX_FREQ_THRESHOLD &&
                    max_mag > MAG_THRESHOLD &&
                    max_mag > relative_cutoff) {

                    oscillators[n].frequency = freq;
                    oscillators[n].phase = 0.0f;
                    oscillators[n].is_active = true;
                    active_count++;

                    printf("Osc[%d]: %.1f Hz (Mag: %.1f)\n", n, freq, max_mag);
                    magnitudes[max_bin] = 0.0f;
                } else {
                    oscillators[n].is_active = false;
                }
            }

            if (active_count > 0) {
                printf("  -> %d/%d active (cutoff: %.1f)\n",
                       active_count, N_FREQ, relative_cutoff);
            }
        }

        // Small delay to prevent excessive button polling
        delay_millis(TIM2, 10);
    }
}
