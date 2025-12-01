// main.c
// Musical Tesla Coil - ADC Threshold Test
// PA6 (ADC input) → PA11 (square wave output)
// LED ON when PA6 voltage > 1.65V, OFF when < 1.65V

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Define NVIC for interrupt control (not in library headers)
typedef struct {
    volatile uint32_t ISER[8];
} NVIC_Type;
#define NVIC ((NVIC_Type *) 0xE000E100UL)

// Project Headers - these define TIM15, TIM_TypeDef, DMA1, DMA_TypeDef, ADC
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

#define AUDIO_INPUT_PIN     6       // PA6 - ADC Channel 11
#define ADC_CHANNEL         11      // ADC1_IN11 on PA6
#define SQUARE_OUT_PIN      11      // PA11 - Square wave output to FPGA
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

    // Configure PA6 as analog for ADC
    pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);

    // Configure PA11 as output with explicit register settings
    // MODER: 01 = General purpose output mode
    GPIOA->MODER &= ~(3U << (SQUARE_OUT_PIN * 2));  // Clear bits
    GPIOA->MODER |= (1U << (SQUARE_OUT_PIN * 2));   // Set to output (01)

    // OTYPER: 0 = Push-pull (NOT open-drain)
    GPIOA->OTYPER &= ~(1U << SQUARE_OUT_PIN);

    // OSPEEDR: 11 = Very high speed
    GPIOA->OSPEEDR |= (3U << (SQUARE_OUT_PIN * 2));

    // PURPDR: 00 = No pull-up, no pull-down (library has typo: PURPDR not PUPDR)
    GPIOA->PURPDR &= ~(3U << (SQUARE_OUT_PIN * 2));

    // Start with output LOW
    GPIOA->ODR &= ~(1U << SQUARE_OUT_PIN);

    printf("System initialized\n");
    printf("RCC APB2ENR: %lu\n", RCC->APB2ENR);
    printf("RCC CFGR: %lu\n", RCC->CFGR);
    printf("RCC AHB2ENR: %lu\n", RCC->AHB2ENR);
    printf("GPIOA MODER[PA11]: %lu\n", (GPIOA->MODER >> (SQUARE_OUT_PIN * 2)) & 3);
    printf("GPIOA OTYPER[PA11]: %lu\n", (GPIOA->OTYPER >> SQUARE_OUT_PIN) & 1);
}

void initADC_Manual(uint8_t channel) {
    // Following STM32L432KC reference manual sequence exactly
    printf("Starting ADC init...\n");

    // 1. Enable ADC clock
    RCC->AHB2ENR |= (1 << 13);  // ADCEN bit
    printf("1. ADC clock enabled\n");

    // 2. Configure ADC clock prescaler (default: /1)
    ADC_COMMON->CCR = 0;  // CKMODE = 00 (use default)
    printf("2. Clock prescaler configured\n");

    // 3. Ensure ADC is disabled before calibration
    printf("3. Checking if ADC needs disabling...\n");
    if (ADC1->CR & (1 << 0)) {  // ADEN
        printf("   ADC is enabled, disabling...\n");
        ADC1->CR |= (1 << 1);   // ADDIS - disable ADC
        while (ADC1->CR & (1 << 0));  // Wait until ADEN = 0
        printf("   ADC disabled\n");
    } else {
        printf("   ADC already disabled\n");
    }

    // 3.5. Enable ADC voltage regulator (CRITICAL for STM32L4!)
    printf("3.5. Enabling ADC voltage regulator...\n");
    ADC1->CR &= ~(0b11 << 28);  // Clear ADVREGEN bits [29:28]
    ADC1->CR |= (0b01 << 28);   // ADVREGEN = 01 (enable regulator)
    // Wait for voltage regulator to stabilize (tADCVREG_STUP = 20 µs typical)
    for (volatile int i = 0; i < 2000; i++);  // ~20 µs at 80 MHz
    printf("     Voltage regulator stable\n");

    // 4. Calibrate ADC (single-ended mode)
    printf("4. Starting calibration (single-ended mode)...\n");
    ADC1->CR &= ~(1 << 30);  // ADCALDIF = 0 (single-ended calibration)
    ADC1->CR |= (1 << 31);   // ADCAL = 1 (start calibration)
    printf("   Calibration started, waiting...\n");

    // Add timeout to prevent infinite loop
    int timeout = 100000;
    while ((ADC1->CR & (1 << 31)) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        printf("   WARNING: Calibration timeout!\n");
    } else {
        printf("   Calibration complete\n");
    }

    // 5. Clear ADRDY flag
    ADC1->ISR |= (1 << 0);  // Clear ADRDY
    printf("5. ADRDY flag cleared\n");

    // 6. Enable ADC
    printf("6. Enabling ADC...\n");
    ADC1->CR |= (1 << 0);  // ADEN
    while (!(ADC1->ISR & (1 << 0)));  // Wait for ADRDY
    printf("   ADC enabled and ready\n");

    // 7. Configure: 12-bit, right align, single conversion
    ADC1->CFGR = 0;  // All defaults (12-bit, right align, software trigger)
    printf("7. Configuration register set\n");

    // 8. Select channel in SQR1 (first/only conversion)
    ADC1->SQR1 = (channel << 6);  // SQ1 bits [10:6]
    printf("8. Channel %d selected\n", channel);

    // 9. Set sampling time for the channel
    // Channel 11 uses SMPR2 bits [5:3]
    if (channel <= 9) {
        // Channels 0-9 use SMPR1
        ADC1->SMPR1 &= ~(7U << (channel * 3));
        ADC1->SMPR1 |= (4U << (channel * 3));  // 47.5 cycles
    } else {
        // Channels 10-18 use SMPR2
        ADC1->SMPR2 &= ~(7U << ((channel - 10) * 3));
        ADC1->SMPR2 |= (4U << ((channel - 10) * 3));  // 47.5 cycles
    }
    printf("9. Sampling time configured\n");

    printf("ADC manually initialized for channel %d\n", channel);
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

    // Initialize ADC with manual bare-metal sequence
    initADC_Manual(ADC_CHANNEL);

    // Threshold: 2048 = midpoint of 12-bit ADC (1.65V with 3.3V reference)
    const uint16_t THRESHOLD = 2048;

    printf("\n===== ADC Threshold Test =====\n");
    printf("Reading PA6 (ADC1_IN11) analog input\n");
    printf("Square wave output on PA11\n");
    printf("PA11 HIGH when PA6 voltage > 1.65V\n");
    printf("PA11 LOW when PA6 voltage < 1.65V\n\n");
    printf("With 500 Hz sine wave, PA11 should toggle at 500 Hz\n\n");

    // First test: Turn LED ON for 1 second to verify GPIO works
    printf("Testing PA11 GPIO: LED should turn ON for 1 second...\n");
    digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
    for (volatile int i = 0; i < 8000000; i++);  // ~1s delay
    digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
    printf("GPIO test done. Starting ADC polling...\n\n");

    // Main loop - simple threshold detection with debug output
    int loop_count = 0;
    while (1) {
        // Start ADC conversion
        ADC1->CR |= (1 << 2);  // ADSTART

        // Wait for conversion to complete
        while (!(ADC1->ISR & (1 << 2)));  // Wait for EOC (End Of Conversion)

        // Read ADC value (12-bit: 0-4095)
        uint16_t adc_value = ADC1->DR;

        // Print ADC value every 10000 iterations
        if (loop_count % 10000 == 0) {
            float voltage = (adc_value * 3.3f) / 4095.0f;
            printf("ADC: %u (%.3f V) | Threshold: %u | Output: %s\n",
                   adc_value, voltage, THRESHOLD,
                   (adc_value > THRESHOLD) ? "HIGH" : "LOW");
        }

        // Simple threshold comparison
        if (adc_value > THRESHOLD) {
            digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);  // Voltage > 1.65V
        } else {
            digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);   // Voltage < 1.65V
        }

        loop_count++;
    }

    return 0;
}

// ============================================================================
// FFT-based Frequency Detection with LED (commented out)
// ============================================================================
// int main(void) {
//     initSystem();
//     initFFT();
//     configureADCForDMA(ADC_CHANNEL);
//     initDMA_ADC(adc_buffer, BUFFER_SIZE);
//     NVIC->ISER[0] |= (1 << 11);  // DMA1_Channel1_IRQn
//     enableDMA_ADC();
//     startADC();
//     const float TARGET_FREQ_MIN = 400.0f;
//     const float TARGET_FREQ_MAX = 600.0f;
//     while (1) {
//         if (buffer_ready) {
//             buffer_ready = false;
//             processFFT(adc_buffer, top_frequencies);
//             float dominant_freq = top_frequencies[0].frequency;
//             if (dominant_freq >= TARGET_FREQ_MIN && dominant_freq <= TARGET_FREQ_MAX) {
//                 digitalWrite(SQUARE_OUT_PIN, GPIO_HIGH);
//             } else {
//                 digitalWrite(SQUARE_OUT_PIN, GPIO_LOW);
//             }
//         }
//     }
//     return 0;
// }
// ============================================================================

// ============================================================================
// GPIO Passthrough Test (commented out - WORKING)
// ============================================================================
// int main(void) {
//     initSystem();
//     pinMode(AUDIO_INPUT_PIN, GPIO_INPUT);
//     printf("\n===== GPIO Passthrough Test =====\n");
//     printf("PA11 (input) → PA6 (LED output)\n");
//     printf("Testing if GPIO input is reading correctly\n\n");
//     while (1) {
//         int input_state = digitalRead(AUDIO_INPUT_PIN);
//         digitalWrite(SQUARE_OUT_PIN, input_state);
//     }
//     return 0;
// }
// ============================================================================

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
