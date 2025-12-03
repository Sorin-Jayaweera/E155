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
#include "DFPLAYER_MINI.h"

// Forward declarations for USART (avoid including header that conflicts with custom libs)
#define USART1_ID 1
USART_TypeDef * initUSART(int USART_ID, int baud_rate);

/*******************************************************************************
 * CONFIGURATION PARAMETERS
 ******************************************************************************/

// Pin Definitions
// FFT Tesla Coil (REMAPPED to avoid DFPlayer conflicts)
#define LED_PIN         0       // PB0 (moved from PA9) - Square wave output to Tesla coil
#define AUDIO_INPUT_PIN 5       // PA5 (moved from PA6) - Analog audio input from DFPlayer
#define ADC_CHANNEL     10      // ADC1 Channel 10 (maps to PA5)

// DFPlayer Mini Pin Definitions (ORIGINAL working configuration)
#define USART_TX_PIN    9       // PA9 - USART1 TX to DFPlayer RX
#define USART_RX_PIN    10      // PA10 - USART1 RX from DFPlayer TX
#define BTN_PREVIOUS    8       // PA8 - Previous track button
#define BTN_PAUSE       6       // PA6 - Pause/Play button (original)
#define BTN_NEXT        7       // PB7 - Next track button (original)

// Signal Processing Parameters
#define FFT_SIZE        256     // FFT window size (must be power of 2)
#define SAMPLE_RATE     8000    // Sampling frequency in Hz

// Detection Thresholds
#define FREQ_THRESHOLD      40.0f    // Minimum frequency (Hz) - lowered for bass frequencies
#define MAX_FREQ_THRESHOLD  2000.0f  // Maximum frequency (Hz) - filters out high harmonics
#define MAG_THRESHOLD       10.0f    // Minimum absolute magnitude to avoid noise
#define RELATIVE_MAG_THRESHOLD 0.5f  // Min magnitude relative to strongest (0.0-1.0)
                                      // Higher = fewer frequencies (stricter)
                                      // Lower = more frequencies (allows harmonics)
                                      // Recommended: 0.5-0.7 for music
// Useful frequency range for music: 150 Hz - 2000 Hz
// 150 Hz ≈ D3, 2000 Hz ≈ B6 (covers most musical instruments)

// ============================================================================
// ADC TRIGGER CONFIGURATION - CHANGE THIS TO TEST DIFFERENT TRIGGER SOURCES
// ============================================================================
// ADC1 External Trigger Selection (EXTSEL) for STM32L432KC
// Reference: STM32L4 Reference Manual RM0394, Table 72
//
// EXTSEL Value | Trigger Source      | Description
// -------------|---------------------|----------------------------------------
//      0       | TIM1_CC1            | Timer 1 Capture/Compare 1
//      1       | TIM1_CC2            | Timer 1 Capture/Compare 2
//      2       | TIM1_CC3            | Timer 1 Capture/Compare 3
//      3       | TIM2_CC2            | Timer 2 Capture/Compare 2
//      4       | TIM3_TRGO           | Timer 3 Trigger Output
//      5       | TIM4_CC4            | Timer 4 Capture/Compare 4
//      6       | EXTI line 11        | External interrupt line 11
//      9       | TIM1_TRGO           | Timer 1 Trigger Output
//     10       | TIM1_TRGO2          | Timer 1 Trigger Output 2
//     11       | TIM2_TRGO           | Timer 2 Trigger Output
//     12       | TIM4_TRGO           | Timer 4 Trigger Output
//     13       | TIM6_TRGO           | Timer 6 Trigger Output (EXPECTED)
//     14       | TIM15_TRGO          | Timer 15 Trigger Output
//     15       | TIM3_CC4            | Timer 3 Capture/Compare 4
//
// CURRENT SETTING: Using TIM6, so EXTSEL should be 13
// IF DMA RATE IS WRONG: Try other timer trigger values (4, 9, 11, 12, 14)
//
// QUICK TEST GUIDE:
// 1. Change ADC_EXTSEL below to a different value
// 2. Rebuild and flash
// 3. Observe DMA interrupt rate and LED toggle speed
// 4. If LED blinks at 5 Hz (~100ms ON/OFF), you found the correct EXTSEL!
// ============================================================================
#define ADC_EXTSEL      13      // <<< CHANGE THIS VALUE TO TEST DIFFERENT TRIGGERS

/*******************************************************************************
 * HARDWARE REGISTER DEFINITIONS
 * (Missing from library headers - defined here for bare-metal access)
 ******************************************************************************/

// Nested Vectored Interrupt Controller (NVIC) - For enabling interrupts
typedef struct {
    volatile uint32_t ISER[8];      // Interrupt Set Enable Register
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// System Control Block (SCB) - For FPU enable
// Note: Only defining CPACR at offset 0x88 from SCB base
#define SCB_CPACR (*((volatile uint32_t *) 0xE000ED88UL))

// TIM1 Advanced Timer (APB2 bus)
#define TIM1_BASE  (0x40012C00UL)
#define TIM1       ((TIM_TypeDef *) TIM1_BASE)

// TIM6 Basic Timer (APB1 bus)
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

// Frequency oscillator for multi-frequency synthesis
typedef struct {
    float frequency;    // Frequency in Hz
    float phase;        // Phase accumulator (0.0 to 1.0)
    bool is_active;     // Whether this oscillator is enabled
} FrequencyOscillator;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

// ADC buffer filled by DMA (raw 12-bit ADC values: 0-4095)
uint16_t adc_buffer[FFT_SIZE];

// Flag set by DMA interrupt when buffer is full and ready for processing
volatile bool buffer_ready = false;

// FFT buffer for frequency domain analysis
Complex fft_buffer[FFT_SIZE];

// Multi-frequency synthesis parameters
#define SYNTHESIS_RATE 100000  // 100 kHz synthesis update rate
#define MAX_OSCILLATORS 5      // Maximum simultaneous frequencies
#define N_FREQ 5               // Number of top frequencies to synthesize (CHANGE THIS!)

// Array of frequency oscillators for synthesis
FrequencyOscillator oscillators[MAX_OSCILLATORS];

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
        // Clear ALL Channel 1 flags (GIF1, TCIF1, HTIF1, TEIF1) to prevent infinite loop
        // Bits 0-3: CGIF1, CTCIF1, CHTIF1, CTEIF1
        DMA1->IFCR |= (0xF << 0);

        // Signal main loop that buffer is ready for FFT processing
        buffer_ready = true;
    }
}


/*******************************************************************************
 * HARDWARE INITIALIZATION FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize GPIO and FPU
 *
 * CONFIGURATION:
 *   FFT Tesla Coil (REMAPPED):
 *     - PA5: Analog input (ADC1_IN10) - moved from PA6
 *     - PB0: Digital output (square wave to Tesla coil) - moved from PA9
 *   DFPlayer Mini (ORIGINAL):
 *     - PA9/PA10: USART1 TX/RX (hardware USART to DFPlayer)
 *     - PA8: Previous button (input, external pull-down, active HIGH)
 *     - PA6: Pause/Play button (input, external pull-down, active HIGH)
 *     - PB7: Next button (input, external pull-down, active HIGH)
 *   - Enables Floating Point Unit (FPU) for fast math operations
 *
 * NOTE: Flash and system clock must be configured BEFORE calling this!
 *       Buttons use EXTERNAL pull-down resistors (no internal pulls needed)
 *       USART1 will be initialized separately in main()
 */
void initSystem(void) {
    // Enable FPU FIRST (before any floating point operations)
    // CP10 and CP11 coprocessor access: 11 = Full access
    // Bits [21:20] = CP10, Bits [23:22] = CP11
    SCB_CPACR |= ((3UL << 10*2) | (3UL << 11*2));

    // Enable GPIOA and GPIOB clocks (AHB2 bus)
    // Bit 0: GPIOAEN, Bit 1: GPIOBEN
    RCC->AHB2ENR |= (1 << 0) | (1 << 1);

    // Configure FFT Tesla Coil pins (REMAPPED)
    // LED_PIN is now PB0 (not PA9)
    GPIOB->MODER &= ~(0b11 << (LED_PIN * 2));
    GPIOB->MODER |= (0b01 << (LED_PIN * 2));  // PB0 as output

    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);     // PA5 as analog input

    // Configure DFPlayer button pins (NO internal pulls - external pull-downs)
    pinMode(BTN_PREVIOUS, GPIO_INPUT);         // PA8 as input
    GPIOA->PURPDR &= ~(0b11 << (BTN_PREVIOUS * 2));  // No pull

    pinMode(BTN_PAUSE, GPIO_INPUT);            // PA6 as input
    GPIOA->PURPDR &= ~(0b11 << (BTN_PAUSE * 2));     // No pull

    // PB7 (GPIOB pin) for Next button
    GPIOB->MODER &= ~(0b11 << (BTN_NEXT * 2));       // PB7 input
    GPIOB->PURPDR &= ~(0b11 << (BTN_NEXT * 2));      // No pull
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
 * @brief TIM15 ISR - Multi-frequency synthesis via phase accumulator
 *
 * OPERATION:
 *   - Called at SYNTHESIS_RATE (100 kHz)
 *   - Updates phase accumulator for each active oscillator
 *   - OR's all oscillator outputs together
 *   - Writes result to PA9 (SQUARE_OUT_PIN)
 *
 * PHASE ACCUMULATOR:
 *   phase += frequency / SYNTHESIS_RATE
 *   When phase >= 1.0, wrap to phase - 1.0
 *   Output HIGH when phase < 0.5 (50% duty cycle)
 */
void TIM1_BRK_TIM15_IRQHandler(void) {
    if (TIM15->SR & (1 << 0)) {  // UIF - Update Interrupt Flag
        TIM15->SR &= ~(1 << 0);   // Clear flag

        bool output_state = false;

        // Update all oscillators and OR their outputs
        for (int i = 0; i < MAX_OSCILLATORS; i++) {
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

        // Update GPIO (PB0 - use direct register access)
        if (output_state) {
            GPIOB->BSRR = (1 << LED_PIN);  // Set PB0 high
        } else {
            GPIOB->BSRR = (1 << (LED_PIN + 16));  // Set PB0 low
        }
    }
}

/**
 * @brief Initialize TIM15 for multi-frequency synthesis
 *
 * CONFIGURATION:
 *   Timer: TIM15 (APB2 bus, 80 MHz)
 *   Update Rate: 100 kHz (SYNTHESIS_RATE)
 *   Mode: Interrupt-driven phase accumulator synthesis
 *
 * CALCULATION:
 *   TIM15_CLK = 80 MHz
 *   PSC = 0 (no prescaler)
 *   ARR = 800 - 1 = 799
 *   Update Rate = 80 MHz / 800 = 100 kHz
 */
void initTIM15_Synthesis(void) {
    // Enable TIM15 clock (APB2 bus)
    RCC->APB2ENR |= (1 << 16);  // TIM15EN

    // Configure for 100 kHz interrupt rate
    TIM15->PSC = 0;             // No prescaler (80 MHz timer clock)
    TIM15->ARR = 800 - 1;       // 80 MHz / 800 = 100 kHz

    // Enable update interrupt
    TIM15->DIER |= (1 << 0);    // UIE = 1

    // Enable TIM15 interrupt in NVIC (TIM1_BRK_TIM15_IRQn = 24)
    NVIC->ISER[0] |= (1 << 24);

    // Start timer
    TIM15->CR1 |= (1 << 0);     // CEN = 1
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

    // Select external trigger source
    // EXTSEL[3:0] bits [9:6] - Configurable via ADC_EXTSEL define at top of file
    ADC1->CFGR &= ~(0xF << 6);              // Clear EXTSEL bits
    ADC1->CFGR |= (ADC_EXTSEL << 6);        // Set EXTSEL from configuration

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
    // Configure Flash wait states FIRST (required for 80 MHz operation)
    // CRITICAL: Must be done before configureClock() or CPU will crash!
    configureFlash();

    // Configure system clock to 80 MHz using PLL
    // CRITICAL: Must be called after configureFlash()!
    configureClock();

    // Initialize all hardware subsystems
    initSystem();        // GPIO, FPU (clock and flash already configured above)
    initADC_DMA();       // ADC and DMA (MUST be before timer!)
    initTimer_ADC();     // TIM6 trigger at 8 kHz

    // Initialize USART1 for DFPlayer Mini (PA9/PA10, 9600 baud)
    USART_TypeDef * USART1 = initUSART(USART1_ID, 9600);

    printf("\n========================================\n");
    printf("  FFT VALIDATION MODE\n");
    printf("========================================\n");
    printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    printf("FFT Size: %d samples\n", FFT_SIZE);
    printf("Frequency Resolution: %.2f Hz/bin\n", (float)SAMPLE_RATE / FFT_SIZE);
    printf("Update Rate: %.1f Hz\n", (float)SAMPLE_RATE / FFT_SIZE);
    printf("\nLED ON: Frequency > %.0f Hz\n", FREQ_THRESHOLD);
    printf("LED OFF: Frequency < %.0f Hz\n", FREQ_THRESHOLD);
    printf("\n========================================\n");
    printf("  ADC TRIGGER CONFIGURATION\n");
    printf("========================================\n");
    printf("ADC_EXTSEL = %d\n", ADC_EXTSEL);
    printf("Expected: 13 for TIM6_TRGO\n");
    printf("If DMA rate is wrong, try: 4, 9, 11, 12, or 14\n");
    printf("========================================\n\n");

    // =========================================================================
    // =========================================================================
    // FFT-BASED FREQUENCY SYNTHESIS TEST
    // =========================================================================
    // Full pipeline test: ADC input → FFT → Frequency detection → Synthesis
    //
    // Test procedure:
    //   1. Feed sine wave into PA6 (e.g., 50 Hz, 320 Hz, 1000 Hz)
    //   2. FFT detects dominant frequency
    //   3. PA9 outputs square wave at detected frequency
    //
    // Expected: Input frequency = Output frequency
    // =========================================================================
    printf("***** FFT-BASED FREQUENCY SYNTHESIS TEST *****\n");
    printf("Feed sine wave into PA6 (A5)\n");
    printf("PA9 will output square wave at detected frequency\n");
    printf("Frequency range: 20 Hz - 2000 Hz\n");
    printf("Update rate: ~31 Hz (every 256 samples)\n\n");

    // Initialize all oscillators to inactive
    for (int i = 0; i < MAX_OSCILLATORS; i++) {
        oscillators[i].frequency = 0.0f;
        oscillators[i].phase = 0.0f;
        oscillators[i].is_active = false;
    }

    printf("Starting TIM15 synthesis engine...\n");
    initTIM15_Synthesis();

    printf("\n========================================\n");
    printf("  DFPLAYER MINI INITIALIZATION\n");
    printf("========================================\n");
    printf("Initializing DFPlayer...\n");
    DF_Init(USART1, 15);  // Volume: 15/30 (50%)
    printf("DFPlayer ready!\n");
    printf("  PA8: Previous track\n");
    printf("  PA6: Pause/Play\n");
    printf("  PB7: Next track\n");
    printf("Starting playback...\n");
    DF_PlayFromStart(USART1);
    printf("========================================\n\n");

    printf("Starting FFT processing loop...\n");
    printf("Waiting for audio input...\n\n");

    // Main processing loop: FFT → Synthesis + DFPlayer control
    while(1) {
        // Check DFPlayer control buttons (from original DFPLAYER_MINI.c)
        Check_Key(USART1);

        if (buffer_ready) {
            buffer_ready = false;

            // STEP 1: Convert ADC samples to normalized complex numbers
            for (int i = 0; i < FFT_SIZE; i++) {
                fft_buffer[i].real = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
                fft_buffer[i].imag = 0.0f;
            }

            // STEP 2: Perform FFT
            fft_compute(fft_buffer, FFT_SIZE);

            // STEP 3: Calculate magnitude for all frequency bins
            float magnitudes[FFT_SIZE / 2];
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                float real = fft_buffer[i].real;
                float imag = fft_buffer[i].imag;
                magnitudes[i] = sqrtf(real * real + imag * imag);
            }

            // STEP 4: Find global maximum to set relative threshold
            float global_max_mag = 0.0f;
            for (int i = 1; i < FFT_SIZE / 2; i++) {
                if (magnitudes[i] > global_max_mag) {
                    global_max_mag = magnitudes[i];
                }
            }

            // Calculate relative magnitude cutoff
            float relative_cutoff = global_max_mag * RELATIVE_MAG_THRESHOLD;

            // STEP 5: Find top N_FREQ frequencies with relative magnitude filtering
            int active_count = 0;
            for (int n = 0; n < N_FREQ; n++) {
                // Find the bin with maximum magnitude (that hasn't been used)
                float max_mag = 0.0f;
                int max_bin = 0;

                for (int i = 1; i < FFT_SIZE / 2; i++) {
                    if (magnitudes[i] > max_mag) {
                        max_mag = magnitudes[i];
                        max_bin = i;
                    }
                }

                // Convert bin to frequency
                float freq = (float)max_bin * SAMPLE_RATE / FFT_SIZE;

                // Check if this frequency is valid:
                // 1. Within frequency range (FREQ_THRESHOLD < freq < MAX_FREQ_THRESHOLD)
                // 2. Above absolute magnitude threshold (MAG_THRESHOLD)
                // 3. Above relative magnitude threshold (relative_cutoff)
                if (freq > FREQ_THRESHOLD &&
                    freq < MAX_FREQ_THRESHOLD &&
                    max_mag > MAG_THRESHOLD &&
                    max_mag > relative_cutoff) {

                    // Valid frequency - add to oscillator array
                    oscillators[n].frequency = freq;
                    oscillators[n].phase = 0.0f;
                    oscillators[n].is_active = true;
                    active_count++;

                    printf("Osc[%d]: %.1f Hz (Mag: %.1f)\n", n, freq, max_mag);

                    // Mark this bin as used (set magnitude to 0)
                    magnitudes[max_bin] = 0.0f;
                } else {
                    // Frequency rejected (out of range, too weak, or harmonic)
                    // Deactivate this oscillator
                    oscillators[n].is_active = false;
                }
            }

            // Print summary
            if (active_count > 0) {
                printf("  -> %d/%d active (cutoff: %.1f)\n",
                       active_count, N_FREQ, relative_cutoff);
            } else {
                // All oscillators inactive - no output
            }
        }
    }
}  // End main()
