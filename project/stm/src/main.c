/*******************************************************************************
 * main.c - STM32L432KC FFT Audio Frequency Analyzer
 *
 * PURPOSE:
 *   Musical Tesla Coil controller that:
 *   1. Samples audio input via ADC (8 kHz sample rate)
 *   2. Performs Fast Fourier Transform (FFT) to identify dominant frequency
 *   3. Outputs LED indication when frequency > 100 Hz is detected
 *
 * HARDWARE CONFIGURATION:
 *   - Input:  PA6 (Board Label: A5, ADC Channel 11)
 *   - Output: PA9 (Board Label: D1, LED indicator)
 *   - Platform: STM32L432KC Nucleo-32
 *   - Reference Voltage: 3.3V
 *
 * SIGNAL PROCESSING PIPELINE:
 *   TIM6 (8kHz) → ADC1 → DMA1_Ch1 → FFT → Frequency Detection → LED Output
 *
 * KEY PARAMETERS:
 *   - Sample Rate: 8000 Hz (allows detection up to 4 kHz via Nyquist theorem)
 *   - FFT Size: 256 samples
 *   - Frequency Resolution: 31.25 Hz per bin (8000 Hz / 256)
 *   - Update Rate: ~31 Hz (8000 Hz / 256 samples)
 *
 * AUTHOR: Musical Tesla Coil Project
 * DATE: 2024
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

// Project Libraries
#include "../lib/STM32L432KC_RCC.h"
#include "../lib/STM32L432KC_GPIO.h"
#include "../lib/STM32L432KC_ADC.h"
#include "../lib/STM32L432KC_TIM.h"
#include "../lib/STM32L432KC_DMA.h"
#include "../lib/STM32L432KC_FLASH.h"

/*******************************************************************************
 * CONFIGURATION PARAMETERS
 ******************************************************************************/

// Pin Definitions
#define LED_PIN         9       // PA9 (Board D1) - Output LED indicator
#define AUDIO_INPUT_PIN 6       // PA6 (Board A5) - Analog audio input
#define ADC_CHANNEL     11      // ADC1 Channel 11 (maps to PA6)

// Signal Processing Parameters
#define FFT_SIZE        256     // FFT window size (must be power of 2)
#define SAMPLE_RATE     8000    // Sampling frequency in Hz

// Detection Thresholds
#define FREQ_THRESHOLD  100.0f  // Minimum frequency to trigger LED (Hz)
#define MAG_THRESHOLD   10.0f   // Minimum magnitude to avoid noise

/*******************************************************************************
 * HARDWARE REGISTER DEFINITIONS
 * (Missing from library headers - defined here for bare-metal access)
 ******************************************************************************/

// Nested Vectored Interrupt Controller (NVIC) - For enabling interrupts
typedef struct {
    volatile uint32_t ISER[8];      // Interrupt Set Enable Register
    uint32_t R0[24];                // Reserved
    volatile uint32_t ICER[8];      // Interrupt Clear Enable Register
    uint32_t R1[24];                // Reserved
    volatile uint32_t ISPR[8];      // Interrupt Set Pending Register
    uint32_t R2[24];                // Reserved
    volatile uint32_t ICPR[8];      // Interrupt Clear Pending Register
    uint32_t R3[24];                // Reserved
    volatile uint32_t IABR[8];      // Interrupt Active Bit Register
    uint32_t R4[56];                // Reserved
    volatile uint8_t IP[240];       // Interrupt Priority Register
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// System Control Block (SCB) - For FPU enable
typedef struct {
    volatile const uint32_t CPUID;  // CPU ID Base Register
    volatile uint32_t ICSR;         // Interrupt Control and State Register
    volatile uint32_t VTOR;         // Vector Table Offset Register
    volatile uint32_t AIRCR;        // Application Interrupt and Reset Control
    volatile uint32_t SCR;          // System Control Register
    volatile uint32_t CCR;          // Configuration and Control Register
    volatile uint8_t SHP[12];       // System Handler Priority Register
    volatile uint32_t SHCSR;        // System Handler Control and State
    volatile uint32_t CFSR;         // Configurable Fault Status Register
    volatile uint32_t HFSR;         // HardFault Status Register
    volatile uint32_t DFSR;         // Debug Fault Status Register
    volatile uint32_t MMFAR;        // MemManage Fault Address Register
    volatile uint32_t BFAR;         // BusFault Address Register
    volatile uint32_t AFSR;         // Auxiliary Fault Status Register
    volatile uint32_t PFR[2];       // Processor Feature Register
    volatile uint32_t DFR;          // Debug Feature Register
    volatile uint32_t ADR;          // Auxiliary Feature Register
    volatile uint32_t MMFR[4];      // Memory Model Feature Register
    volatile uint32_t ISAR[5];      // Instruction Set Attributes Register
    volatile uint32_t R0[5];        // Reserved
    volatile uint32_t CPACR;        // Coprocessor Access Control Register
} SCB_Type;
#define SCB ((SCB_Type *) 0xE000ED00UL)

// TIM6 Basic Timer
#define TIM6_BASE  (0x40001000UL)
#define TIM6       ((TIM_TypeDef *) TIM6_BASE)

// Math Constant
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/*******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

// Complex number for FFT computation
typedef struct {
    float real;     // Real component
    float imag;     // Imaginary component
} Complex;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

// ADC buffer filled by DMA (raw 12-bit ADC values: 0-4095)
uint16_t adc_buffer[FFT_SIZE];

// Flag set by DMA interrupt when buffer is full and ready for processing
volatile bool buffer_ready = false;

// FFT buffer for frequency domain analysis
Complex fft_buffer[FFT_SIZE];

/*******************************************************************************
 * FAST FOURIER TRANSFORM (FFT) IMPLEMENTATION
 * Algorithm: Cooley-Tukey Radix-2 Decimation-in-Time FFT
 * Complexity: O(N log N) where N = FFT_SIZE
 ******************************************************************************/

/**
 * @brief Performs in-place Fast Fourier Transform
 * @param data Pointer to complex data array (length n, must be power of 2)
 * @param n Number of samples (must be power of 2)
 *
 * ALGORITHM:
 *   1. Bit-reversal permutation: Reorder input for in-place computation
 *   2. Cooley-Tukey butterfly operations: Combine frequency components
 *
 * MATHEMATICAL BASIS:
 *   X[k] = Σ(n=0 to N-1) x[n] * e^(-j*2π*k*n/N)
 *   where X[k] is the frequency domain representation
 */
void fft_compute(Complex* data, int n) {
    int i, j;

    // STEP 1: Bit-Reversal Permutation
    // Reorders array elements so FFT can be computed in-place
    // Example (n=8): [0,1,2,3,4,5,6,7] → [0,4,2,6,1,5,3,7]
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;

        // Calculate bit-reversed index
        for (; j >= bit; bit >>= 1) {
            j -= bit;
        }
        j += bit;

        // Swap elements if needed
        if (i < j) {
            Complex temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }

    // STEP 2: Cooley-Tukey FFT Butterfly Operations
    // Process in stages: pairs, then groups of 4, 8, 16, etc.
    for (int len = 2; len <= n; len <<= 1) {
        // Calculate twiddle factor for this stage
        // w = e^(-j*2π/len) = cos(-2π/len) + j*sin(-2π/len)
        float angle = -2.0f * M_PI / len;
        Complex wlen = {cosf(angle), sinf(angle)};

        // Process each group of size 'len'
        for (i = 0; i < n; i += len) {
            Complex w = {1.0f, 0.0f};  // w^0 = 1

            // Butterfly operations within this group
            for (j = 0; j < len / 2; j++) {
                // Extract the two elements for butterfly
                Complex u = data[i + j];

                // Complex multiplication: v = data[i + j + len/2] * w
                Complex v = {
                    data[i + j + len/2].real * w.real - data[i + j + len/2].imag * w.imag,
                    data[i + j + len/2].real * w.imag + data[i + j + len/2].imag * w.real
                };

                // Butterfly combination
                data[i + j].real = u.real + v.real;
                data[i + j].imag = u.imag + v.imag;
                data[i + j + len/2].real = u.real - v.real;
                data[i + j + len/2].imag = u.imag - v.imag;

                // Update twiddle factor: w = w * wlen
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

/**
 * @brief DMA1 Channel 1 Transfer Complete Interrupt
 *
 * TRIGGER: When DMA has filled the entire adc_buffer (256 samples)
 * FREQUENCY: ~31 Hz (8000 Hz sample rate / 256 samples)
 * ACTION: Sets buffer_ready flag for main loop to process
 */
void DMA1_Channel1_IRQHandler(void) {
    // Check if Transfer Complete flag is set (TCIF1, bit 1)
    if (DMA1->ISR & (1 << 1)) {
        // Clear the flag by writing 1 to CTCIF1 (bit 1)
        DMA1->IFCR |= (1 << 1);

        // Signal main loop that buffer is ready for FFT processing
        buffer_ready = true;
    }
}

/*******************************************************************************
 * HARDWARE INITIALIZATION FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize system clocks and peripherals
 *
 * CONFIGURATION:
 *   - Enables instruction cache, data cache, and prefetch buffer
 *   - Enables GPIOA clock for PA6 and PA9
 *   - Configures GPIO pins for analog input and digital output
 *   - Enables Floating Point Unit (FPU) for fast math operations
 */
void initSystem(void) {
    // Enable Flash performance features for 80 MHz operation
    // Bit 8: ICEN (Instruction cache enable)
    // Bit 9: DCEN (Data cache enable)
    // Bit 10: PRFTEN (Prefetch enable)
    FLASH->ACR |= (1 << 8) | (1 << 9) | (1 << 10);

    // Enable GPIOA clock (AHB2 bus)
    // Bit 0: GPIOAEN
    RCC->AHB2ENR |= (1 << 0);

    // Configure GPIO pins using library functions
    pinMode(LED_PIN, GPIO_OUTPUT);          // PA9 as digital output
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);  // PA6 as analog input

    // Enable FPU (Floating Point Unit) for hardware accelerated math
    // CP10 and CP11 coprocessor access: 11 = Full access
    // Bits [21:20] = CP10, Bits [23:22] = CP11
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
}

/**
 * @brief Initialize TIM6 to trigger ADC at 8 kHz
 *
 * TIMER CALCULATION:
 *   System Clock: 80 MHz (configured by configureClock() in RCC library)
 *   Prescaler: 79 (PSC register, divides by PSC+1 = 80)
 *   Auto-Reload: 124 (ARR register, counts 0 to ARR = 125 counts)
 *   Timer Frequency = 80 MHz / 80 / 125 = 8000 Hz
 *
 * TRIGGER OUTPUT:
 *   CR2.MMS = 010 (Master Mode Selection = Update event)
 *   Generates TRGO signal on each timer update (overflow)
 *   This TRGO connects to ADC1 via EXTSEL = 13 (TIM6_TRGO)
 */
void initTimer_ADC(void) {
    // Enable TIM6 clock (APB1 bus)
    // Bit 4: TIM6EN
    RCC->APB1ENR1 |= (1 << 4);

    // Configure prescaler and auto-reload for 8 kHz
    TIM6->PSC = 79;         // Divide by 80: 80 MHz → 1 MHz
    TIM6->ARR = 125 - 1;    // Count 125 times: 1 MHz → 8 kHz

    // Configure Master Mode Selection (MMS) to output TRGO on update
    // CR2 bits [6:4] = 010 (Update event selected as trigger output)
    TIM6->CR2 = (0x2 << 4);

    // Enable timer (CR1 bit 0: CEN = Counter Enable)
    TIM6->CR1 |= (1 << 0);
}

/**
 * @brief Initialize ADC and DMA for continuous sampling
 *
 * ADC CONFIGURATION:
 *   - Channel: 11 (PA6)
 *   - Trigger: TIM6_TRGO (EXTSEL = 13, hardware trigger)
 *   - DMA: Circular mode (auto-restart after buffer fills)
 *   - Clock: Synchronous HCLK/1 (80 MHz)
 *
 * CRITICAL FIXES FOR STM32L4:
 *   1. Exit deep power-down mode (DEEPPWD = 0) before voltage regulator
 *   2. Enable ADC voltage regulator (ADVREGEN = 1) and wait 20 µs
 *   3. Use EXTSEL = 13 for TIM6_TRGO (not EXTSEL = 4 from other STM32 series)
 *   4. Initialize ADC/DMA BEFORE starting timer to catch first trigger
 *
 * DMA FLOW:
 *   TIM6 overflow (8 kHz) → ADC conversion → DMA writes to adc_buffer
 *   After 256 samples → DMA interrupt fires → buffer_ready = true
 */
void initADC_DMA(void) {
    // Initialize DMA using library function
    // Maps: ADC1->DR → adc_buffer (circular mode, 256 transfers)
    initDMA_ADC(adc_buffer, FFT_SIZE);

    // Enable DMA1 Channel 1 interrupt in NVIC
    // ISER[0] bit 11: DMA1_Channel1_IRQn (IRQ 11)
    NVIC->ISER[0] |= (1 << 11);

    // Enable ADC clock (AHB2 bus)
    // Bit 13: ADCEN
    RCC->AHB2ENR |= (1 << 13);

    // Exit deep power-down and enable voltage regulator
    // CRITICAL: Must exit deep power-down BEFORE enabling regulator on STM32L4
    ADC1->CR &= ~(1 << 29);     // DEEPPWD = 0 (exit deep power-down)
    ADC1->CR |= (1 << 28);      // ADVREGEN = 1 (enable voltage regulator)

    // Wait for voltage regulator to stabilize (~20 µs required)
    // At 80 MHz: 2000 iterations ≈ 25 µs
    for(volatile int i = 0; i < 2000; i++);

    // Configure ADC clock to synchronous mode (CKMODE = 01: HCLK/1)
    // ADC_COMMON->CCR bit 16: CKMODE[0] = 1
    ADC_COMMON->CCR |= (1 << 16);

    // Configure ADC for DMA and hardware trigger
    // Bit 0: DMAEN = 1 (DMA enable)
    // Bit 1: DMACFG = 1 (DMA circular mode)
    // Bit 10: EXTEN[0] = 1 (External trigger enable on rising edge)
    ADC1->CFGR |= (1 << 0) | (1 << 1) | (1 << 10);

    // Select external trigger source: TIM6_TRGO
    // EXTSEL[3:0] bits [9:6] = 1101 (13 decimal) for TIM6_TRGO on STM32L4
    // NOTE: This differs from other STM32 series! Confirmed in RM0394 Table 98
    ADC1->CFGR &= ~(0xF << 6);      // Clear EXTSEL bits
    ADC1->CFGR |= (13 << 6);        // Set EXTSEL = 13 (TIM6_TRGO)

    // Clear ADRDY flag and enable ADC
    ADC1->ISR |= (1 << 0);          // Write 1 to clear ADRDY
    ADC1->CR |= (1 << 0);           // ADEN = 1 (Enable ADC)

    // Wait for ADC ready flag
    while (!(ADC1->ISR & (1 << 0)));

    // Configure conversion sequence: Channel 11 only, sequence length = 1
    // SQR1 bits [10:6]: SQ1 = 11 (first conversion is channel 11)
    // SQR1 bits [3:0]: L = 0 (sequence length = 1 conversion)
    ADC1->SQR1 = (ADC_CHANNEL << 6);

    // Set sampling time for channel 11 (uses SMPR2)
    // Channel 11: SMPR2 bits [5:3]
    // Value 010 = 12.5 ADC clock cycles sampling time
    ADC1->SMPR2 |= (2U << 3);

    // Start ADC conversions (ADSTART bit 2)
    ADC1->CR |= (1 << 2);

    // Enable DMA requests from ADC
    enableDMA_ADC();
}

/*******************************************************************************
 * MAIN PROGRAM
 ******************************************************************************/

/**
 * @brief Main program loop
 *
 * OPERATION:
 *   1. Initialize hardware (clocks, GPIO, ADC, DMA, Timer)
 *   2. Wait for buffer_ready flag from DMA interrupt
 *   3. Convert ADC samples to normalized complex numbers
 *   4. Perform FFT to transform time domain → frequency domain
 *   5. Find dominant frequency bin
 *   6. Turn LED ON if frequency > 100 Hz and magnitude > threshold
 *
 * SIGNAL FLOW:
 *   Audio Input (PA6) → ADC (8 kHz) → DMA Buffer → FFT →
 *   Frequency Detection → LED Output (PA9)
 *
 * FREQUENCY RESOLUTION:
 *   Each FFT bin represents: SAMPLE_RATE / FFT_SIZE = 8000 / 256 = 31.25 Hz
 *   Bin 0: DC (0 Hz) - ignored
 *   Bin 1: 31.25 Hz
 *   Bin 2: 62.5 Hz
 *   Bin 3: 93.75 Hz
 *   Bin 4: 125 Hz  ← First bin that will trigger LED
 *   ...
 *   Bin 128: 4000 Hz (Nyquist limit)
 */
int main(void) {
    // Initialize all hardware subsystems
    initSystem();        // Clocks, GPIO, FPU
    initADC_DMA();       // ADC and DMA (MUST be before timer!)
    initTimer_ADC();     // TIM6 trigger at 8 kHz

    printf("\n========================================\n");
    printf("  FFT VALIDATION MODE\n");
    printf("========================================\n");
    printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    printf("FFT Size: %d samples\n", FFT_SIZE);
    printf("Frequency Resolution: %.2f Hz/bin\n", (float)SAMPLE_RATE / FFT_SIZE);
    printf("Update Rate: %.1f Hz\n", (float)SAMPLE_RATE / FFT_SIZE);
    printf("\nLED ON: Frequency > %.0f Hz\n", FREQ_THRESHOLD);
    printf("LED OFF: Frequency < %.0f Hz\n\n", FREQ_THRESHOLD);

    // Main processing loop
    while(1) {
        // Wait for DMA interrupt to signal buffer is full
        if (buffer_ready) {
            buffer_ready = false;  // Clear flag

            // STEP 1: Convert ADC samples to normalized complex numbers
            // ADC range: 0-4095 (12-bit)
            // Normalize to: -1.0 to +1.0 (centered at 2048 = 1.65V)
            for (int i = 0; i < FFT_SIZE; i++) {
                fft_buffer[i].real = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
                fft_buffer[i].imag = 0.0f;  // No imaginary component (real signal)
            }

            // STEP 2: Perform FFT
            // Transforms time domain samples → frequency domain components
            fft_compute(fft_buffer, FFT_SIZE);

            // STEP 3: Find dominant frequency
            // Calculate magnitude for each frequency bin and find maximum
            float max_mag = 0.0f;
            int max_bin = 0;

            // Only check bins 1 to FFT_SIZE/2 (skip DC, use Nyquist limit)
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                // Magnitude = sqrt(real² + imag²)
                float real = fft_buffer[i].real;
                float imag = fft_buffer[i].imag;
                float mag = sqrtf(real * real + imag * imag);

                if (mag > max_mag) {
                    max_mag = mag;
                    max_bin = i;
                }
            }

            // Convert bin number to frequency in Hz
            // Frequency = bin_number × (SAMPLE_RATE / FFT_SIZE)
            float freq = (float)max_bin * SAMPLE_RATE / FFT_SIZE;

            // STEP 4: LED Control Logic
            // Turn ON if:
            //   - Frequency > 100 Hz (avoid DC and low-frequency noise)
            //   - Magnitude > 10.0 (avoid background noise)
            if (freq > FREQ_THRESHOLD && max_mag > MAG_THRESHOLD) {
                digitalWrite(LED_PIN, GPIO_HIGH);
                printf("Detected: %d Hz (Mag: %d) -> LED ON\n",
                       (int)freq, (int)max_mag);
            } else {
                digitalWrite(LED_PIN, GPIO_LOW);
                // No print for OFF state to reduce UART traffic
            }
        }
    }

    return 0;
}
