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
#define FREQ_THRESHOLD      100.0f   // Minimum frequency (Hz) - set to 150 for musical range
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
 * SOFTWARE UART FOR DFPLAYER MINI
 * Bit-banged UART TX on PA10 @ 9600 baud (no RX needed)
 * Replaces hardware USART for pin compatibility with FFT code
 ******************************************************************************/

#define SOFT_UART_TX_PIN 10  // PA10
#define BAUD_RATE 9600
#define BIT_DELAY_US (1000000 / BAUD_RATE)  // ~104 us per bit @ 9600 baud

/**
 * @brief Microsecond delay for software UART bit timing
 * @param us Microseconds to delay
 * @note Calibrated for 80 MHz system clock
 */
static void delay_us(uint32_t us) {
    // At 80 MHz: 80 cycles per microsecond
    // Account for loop overhead (~4 cycles per iteration)
    volatile uint32_t cycles = us * 20;  // Approximate: 80/4 = 20
    while (cycles--);
}

/**
 * @brief Send one byte via software UART (8N1 format)
 * @param byte Byte to transmit
 * @note Replaces sendChar(USART, byte) from E155 library
 */
static void sendChar(uint8_t byte) {
    // START bit (low)
    digitalWrite(SOFT_UART_TX_PIN, GPIO_LOW);
    delay_us(BIT_DELAY_US);

    // 8 data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        if (byte & (1 << i)) {
            digitalWrite(SOFT_UART_TX_PIN, GPIO_HIGH);
        } else {
            digitalWrite(SOFT_UART_TX_PIN, GPIO_LOW);
        }
        delay_us(BIT_DELAY_US);
    }

    // STOP bit (high)
    digitalWrite(SOFT_UART_TX_PIN, GPIO_HIGH);
    delay_us(BIT_DELAY_US);
}

/**
 * @brief Millisecond delay using busy-wait loop
 * @param ms Milliseconds to delay
 * @note Replaces delay_millis(TIM15, ms) from E155 library
 *       TIM15 is reserved for FFT synthesis, so we can't use it for delays
 */
static void delay_millis(uint32_t ms) {
    // At 80 MHz: ~80000 cycles per millisecond
    // Account for loop overhead
    volatile uint32_t cycles = ms * 16000;  // Approximate
    while (cycles--);
}

/*******************************************************************************
 * DFPLAYER MINI CONTROL
 * Adapted from working DFPLAYER_MINI.c library
 * Modified: Software UART instead of hardware USART (sendChar now bit-bangs)
 *           Busy-wait delays instead of TIM15 (delay_millis now busy-waits)
 *           Pin remapping: Pause_Key moved from PA6 to PB0
 ******************************************************************************/

// DFPlayer protocol constants (from original DFPLAYER_MINI.c)
#define Start_Byte  0x7E
#define End_Byte    0xEF
#define Version     0xFF
#define Cmd_Len     0x06
#define Feedback    0x00  // No feedback
#define Source      0x02  // TF CARD

// Button pins (remapped for E155 PCB)
#define Previous_Key  8   // PA8 (same as original)
#define Pause_Key     0   // PB0 (moved from PA6 to avoid ADC conflict)
#define Next_Key      7   // PB7 (same as original)

// State variables (from original DFPLAYER_MINI.c)
static int isPaused = 0;
static int isPlaying = 1;

/**
 * @brief Send command to DFPlayer (from original DFPLAYER_MINI.c)
 * @param cmd Command byte
 * @param Parameter1 Parameter 1
 * @param Parameter2 Parameter 2
 * @note Now uses software UART sendChar() instead of hardware USART
 */
static void Send_cmd(uint8_t cmd, uint8_t Parameter1, uint8_t Parameter2) {
    uint16_t Checksum = Version + Cmd_Len + cmd + Feedback + Parameter1 + Parameter2;
    Checksum = 0 - Checksum;

    uint8_t CmdSequence[10] = {
        Start_Byte,
        Version,
        Cmd_Len,
        cmd,
        Feedback,
        Parameter1,
        Parameter2,
        (Checksum >> 8) & 0x00FF,
        (Checksum & 0x00FF),
        End_Byte
    };

    // Send each byte using software UART (sendChar is now bit-banging function above)
    for (int i = 0; i < 10; i++) {
        sendChar(CmdSequence[i]);
    }
}

/**
 * @brief Initialize DFPlayer Mini (from original DFPLAYER_MINI.c)
 * @param volume Initial volume (0-30)
 * @note Modified: Uses busy-wait delay_millis instead of TIM15
 *                 Button init removed (now in initSystem)
 */
void DF_Init(uint8_t volume) {
    // Wait for DFPlayer to boot (from original)
    delay_millis(500);

    // Initialize DFPlayer (from original)
    Send_cmd(0x3F, 0x00, Source);  // Set source to TF card
    delay_millis(200);
    Send_cmd(0x06, 0x00, volume);  // Set volume
    delay_millis(500);
}

/**
 * @brief Play from start (from original DFPLAYER_MINI.c)
 */
void DF_PlayFromStart(void) {
    Send_cmd(0x03, 0x00, 0x01);  // Play first track
    delay_millis(200);
    isPlaying = 1;
    isPaused = 0;
}

/**
 * @brief Next track (from original DFPLAYER_MINI.c)
 */
void DF_Next(void) {
    Send_cmd(0x01, 0x00, 0x00);  // Next track
    delay_millis(200);
}

/**
 * @brief Previous track (from original DFPLAYER_MINI.c)
 */
void DF_Previous(void) {
    Send_cmd(0x02, 0x00, 0x00);  // Previous track
    delay_millis(200);
}

/**
 * @brief Pause playback (from original DFPLAYER_MINI.c)
 */
void DF_Pause(void) {
    Send_cmd(0x0E, 0x00, 0x00);  // Pause
    delay_millis(200);
}

/**
 * @brief Resume playback (from original DFPLAYER_MINI.c)
 */
void DF_Playback(void) {
    Send_cmd(0x0D, 0x00, 0x00);  // Resume playback
    delay_millis(200);
}

/**
 * @brief Set volume (from original DFPLAYER_MINI.c)
 */
void DF_SetVolume(uint8_t volume) {
    Send_cmd(0x06, 0x00, volume);  // Set volume (0-30)
    delay_millis(200);
}

/**
 * @brief Play specific track (from original DFPLAYER_MINI.c)
 */
void DF_PlayTrack(uint8_t track) {
    Send_cmd(0x03, 0x00, track);  // Play specific track
    delay_millis(200);
}

/**
 * @brief Check buttons and send DFPlayer commands (from original DFPLAYER_MINI.c)
 * @note Modified: PB0/PB7 use direct GPIOB register reads instead of digitalRead
 *                 Uses busy-wait delay_millis instead of TIM15
 */
void Check_Key(void) {
    // Check pause/play button (PB0 instead of PA6)
    uint8_t pause_reading = (GPIOB->IDR >> Pause_Key) & 1;
    if (pause_reading) {
        // Wait for button release (debounce) - from original
        while ((GPIOB->IDR >> Pause_Key) & 1);
        delay_millis(50);  // Additional debounce

        if (isPlaying) {
            isPaused = 1;
            isPlaying = 0;
            DF_Pause();
        } else if (isPaused) {
            isPlaying = 1;
            isPaused = 0;
            DF_Playback();
        }
    }

    // Check previous button (PA8) - from original
    if (digitalRead(Previous_Key)) {
        while (digitalRead(Previous_Key));
        delay_millis(50);
        DF_Previous();
    }

    // Check next button (PB7 instead of digitalRead) - from original
    uint8_t next_reading = (GPIOB->IDR >> Next_Key) & 1;
    if (next_reading) {
        while ((GPIOB->IDR >> Next_Key) & 1);
        delay_millis(50);
        DF_Next();
    }
}

/*******************************************************************************
 * HARDWARE INITIALIZATION FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize GPIO and FPU
 *
 * CONFIGURATION:
 *   FFT Tesla Coil:
 *     - PA6: Analog input (ADC)
 *     - PA9: Digital output (square wave to Tesla coil)
 *   DFPlayer Mini:
 *     - PA10: Software UART TX (output, idle HIGH)
 *     - PA8: Previous button (input with pull-up)
 *     - PB0: Pause/Play button (input with pull-up)
 *     - PB7: Next button (input with pull-up)
 *   - Enables Floating Point Unit (FPU) for fast math operations
 *
 * NOTE: Flash and system clock must be configured BEFORE calling this!
 */
void initSystem(void) {
    // Enable FPU FIRST (before any floating point operations)
    // CP10 and CP11 coprocessor access: 11 = Full access
    // Bits [21:20] = CP10, Bits [23:22] = CP11
    SCB_CPACR |= ((3UL << 10*2) | (3UL << 11*2));

    // Enable GPIOA and GPIOB clocks (AHB2 bus)
    // Bit 0: GPIOAEN, Bit 1: GPIOBEN
    RCC->AHB2ENR |= (1 << 0) | (1 << 1);

    // Configure FFT Tesla Coil pins
    pinMode(LED_PIN, GPIO_OUTPUT);          // PA9 as digital output
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);  // PA6 as analog input

    // Configure DFPlayer software UART pin
    pinMode(SOFT_UART_TX_PIN, GPIO_OUTPUT); // PA10 as digital output
    digitalWrite(SOFT_UART_TX_PIN, GPIO_HIGH);  // UART idle state is HIGH

    // Configure DFPlayer button pins (inputs with pull-up)
    pinMode(BTN_PREVIOUS, GPIO_INPUT);      // PA8 as input
    GPIOA->PUPDR &= ~(0b11 << (BTN_PREVIOUS * 2));
    GPIOA->PUPDR |= (0b01 << (BTN_PREVIOUS * 2));  // Pull-up

    // PB0 and PB7 (GPIOB pins)
    // Set as input mode
    GPIOB->MODER &= ~(0b11 << (BTN_PAUSE * 2));    // PB0 input
    GPIOB->MODER &= ~(0b11 << (BTN_NEXT * 2));     // PB7 input

    // Enable pull-ups
    GPIOB->PUPDR &= ~(0b11 << (BTN_PAUSE * 2));
    GPIOB->PUPDR |= (0b01 << (BTN_PAUSE * 2));     // PB0 pull-up
    GPIOB->PUPDR &= ~(0b11 << (BTN_NEXT * 2));
    GPIOB->PUPDR |= (0b01 << (BTN_NEXT * 2));      // PB7 pull-up
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

        // Update GPIO
        digitalWrite(LED_PIN, output_state ? GPIO_HIGH : GPIO_LOW);
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
    DF_Init(15);  // Volume: 15/30 (50%)
    printf("DFPlayer ready!\n");
    printf("  PA8: Previous track\n");
    printf("  PB0: Pause/Play\n");
    printf("  PB7: Next track\n");
    printf("Starting playback...\n");
    DF_PlayFromStart();
    printf("========================================\n\n");

    printf("Starting FFT processing loop...\n");
    printf("Waiting for audio input...\n\n");

    // Main processing loop: FFT → Synthesis + DFPlayer control
    while(1) {
        // Check DFPlayer control buttons (from original DFPLAYER_MINI.c)
        Check_Key();

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
