// fft_processing.h
// Header for FFT processing functions

#ifndef FFT_PROCESSING_H
#define FFT_PROCESSING_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

// FFT size - power of 2 (256, 512, 1024, etc.)
#define FFT_SIZE        512
#define NUM_FREQUENCIES 5

// Sampling rate in Hz (adjust based on your ADC configuration)
// At 80 MHz with minimum ADC conversion time, we can achieve high sample rates
// For audio: 20 kHz is sufficient to capture up to 10 kHz audio
#define SAMPLE_RATE     20000

///////////////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    float frequency;    // Frequency in Hz
    float magnitude;    // Magnitude of frequency component
} FrequencyPeak;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initFFT(void);
void processFFT(uint16_t* adc_buffer, FrequencyPeak* peaks);
void findTopFrequencies(float* magnitudes, FrequencyPeak* peaks);

#endif
