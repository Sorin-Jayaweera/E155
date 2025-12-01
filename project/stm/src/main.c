// main.c
// FFT SINGLE FILE VALIDATION (Fixed)
// Renamed functions to avoid "Multiply Defined Symbol" errors.

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "../lib/STM32L432KC_RCC.h"
#include "../lib/STM32L432KC_GPIO.h"
#include "../lib/STM32L432KC_ADC.h"
#include "../lib/STM32L432KC_TIM.h"
#include "../lib/STM32L432KC_DMA.h"
#include "../lib/STM32L432KC_FLASH.h"

// --- CONFIGURATION ---
#define LED_PIN         9       // PA9 (D1)
#define AUDIO_INPUT_PIN 6       // PA6 (A5)
#define ADC_CHANNEL     11      // Channel 11
#define FFT_SIZE        256     
#define SAMPLE_RATE     8000    // 8 kHz

// --- MISSING DEFINITIONS PATCH ---
typedef struct { volatile uint32_t ISER[8]; uint32_t R0[24]; volatile uint32_t ICER[8]; uint32_t R1[24]; volatile uint32_t ISPR[8]; uint32_t R2[24]; volatile uint32_t ICPR[8]; uint32_t R3[24]; volatile uint32_t IABR[8]; uint32_t R4[56]; volatile uint8_t IP[240]; } NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)
typedef struct { volatile const uint32_t CPUID; volatile uint32_t ICSR; volatile uint32_t VTOR; volatile uint32_t AIRCR; volatile uint32_t SCR; volatile uint32_t CCR; volatile uint8_t SHP[12]; volatile uint32_t SHCSR; volatile uint32_t CFSR; volatile uint32_t HFSR; volatile uint32_t DFSR; volatile uint32_t MMFAR; volatile uint32_t BFAR; volatile uint32_t AFSR; volatile uint32_t PFR[2]; volatile uint32_t DFR; volatile uint32_t ADR; volatile uint32_t MMFR[4]; volatile uint32_t ISAR[5]; volatile uint32_t R0[5]; volatile uint32_t CPACR; } SCB_Type;
#define SCB ((SCB_Type *) 0xE000ED00UL)
#define TIM6_BASE  (0x40001000UL)
#define TIM6       ((TIM_TypeDef *) TIM6_BASE)
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- GLOBALS ---
uint16_t adc_buffer[FFT_SIZE];
volatile bool buffer_ready = false;

typedef struct { float real; float imag; } Complex_Local;
Complex_Local fft_buffer_local[FFT_SIZE];

// --- FFT MATH FUNCTIONS (Renamed to avoid conflict) ---
void fft_compute_local(Complex_Local* data, int n) {
    int i, j;
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j >= bit; bit >>= 1) j -= bit;
        j += bit;
        if (i < j) { Complex_Local temp = data[i]; data[i] = data[j]; data[j] = temp; }
    }
    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        Complex_Local wlen = {cosf(angle), sinf(angle)};
        for (i = 0; i < n; i += len) {
            Complex_Local w = {1.0f, 0.0f};
            for (j = 0; j < len / 2; j++) {
                Complex_Local u = data[i + j];
                Complex_Local v = { data[i + j + len/2].real * w.real - data[i + j + len/2].imag * w.imag, data[i + j + len/2].real * w.imag + data[i + j + len/2].imag * w.real };
                data[i + j].real = u.real + v.real; data[i + j].imag = u.imag + v.imag;
                data[i + j + len/2].real = u.real - v.real; data[i + j + len/2].imag = u.imag - v.imag;
                float w_temp = w.real;
                w.real = w.real * wlen.real - w.imag * wlen.imag;
                w.imag = w_temp * wlen.imag + w.imag * wlen.real;
            }
        }
    }
}

// --- INTERRUPTS ---
void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & (1 << 1)) {
        DMA1->IFCR |= (1 << 1);
        buffer_ready = true;
    }
}

// --- INIT ---
void initSystem(void) {
    FLASH->ACR |= (1 << 8) | (1 << 9) | (1 << 10);
    RCC->AHB2ENR |= (1 << 0);
    pinMode(LED_PIN, GPIO_OUTPUT);
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); // Enable FPU
}

void initTimer_ADC(void) {
    RCC->APB1ENR1 |= (1 << 4); 
    TIM6->PSC = 79; TIM6->ARR = 125 - 1; // 8kHz
    TIM6->CR2 = (0x2 << 4); TIM6->CR1 |= (1 << 0);
}

void initADC_DMA(void) {
    initDMA_ADC(adc_buffer, FFT_SIZE);
    NVIC->ISER[0] |= (1 << 11);
    RCC->AHB2ENR |= (1 << 13);
    ADC1->CR &= ~(1 << 29); ADC1->CR |= (1 << 28); for(volatile int i=0; i<2000; i++); 
    ADC_COMMON->CCR |= (1 << 16); 
    ADC1->CFGR |= (1 << 0) | (1 << 1) | (1 << 10);
    ADC1->CFGR &= ~(0xF << 6); ADC1->CFGR |= (13 << 6); 
    ADC1->ISR |= (1 << 0); ADC1->CR |= (1 << 0); while (!(ADC1->ISR & (1 << 0))); 
    ADC1->SQR1 = (ADC_CHANNEL << 6); 
    ADC1->SMPR2 |= (2U << 3);     
    ADC1->CR |= (1 << 2);         
    enableDMA_ADC();
}

int main(void) {
    initSystem();
    initADC_DMA();
    initTimer_ADC();
    
    printf("\nFFT VALIDATION MODE (Renamed)\n");

    while(1) {
        if (buffer_ready) {
            buffer_ready = false;
            
            // 1. Convert Buffer to Complex
            for (int i = 0; i < FFT_SIZE; i++) {
                fft_buffer_local[i].real = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
                fft_buffer_local[i].imag = 0.0f;
            }

            // 2. Run FFT (Local version)
            fft_compute_local(fft_buffer_local, FFT_SIZE);

            // 3. Find Dominant Frequency
            float max_mag = 0.0f;
            int max_bin = 0;
            
            // Skip bin 0 (DC)
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                float real = fft_buffer_local[i].real;
                float imag = fft_buffer_local[i].imag;
                float mag = sqrtf(real * real + imag * imag);
                if (mag > max_mag) {
                    max_mag = mag;
                    max_bin = i;
                }
            }
            
            float freq = (float)max_bin * SAMPLE_RATE / FFT_SIZE;
            
            // 4. LED Logic: 
            // If we detect something > 100 Hz, turn ON.
            if (freq > 100.0f && max_mag > 10.0f) {
                digitalWrite(LED_PIN, GPIO_HIGH);
                printf("Freq: %d Hz (Mag: %d) -> LED ON\n", (int)freq, (int)max_mag);
            } else {
                digitalWrite(LED_PIN, GPIO_LOW);
            }
        }
    }
}