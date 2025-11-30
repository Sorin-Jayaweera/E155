// fft_processing.c
// Source code for FFT processing functions

#include "fft_processing.h"
#include "arm_math.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Private variables
///////////////////////////////////////////////////////////////////////////////

// Input and output arrays for FFT
static float fft_input[FFT_SIZE * 2];      // Interleaved real and imaginary (real, imag, real, imag, ...)
static float fft_output[FFT_SIZE];         // Magnitude output
static arm_rfft_fast_instance_f32 fft_instance;

///////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////

void initFFT(void) {
    // Initialize the RFFT (Real FFT) instance
    arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);
}

void processFFT(uint16_t* adc_buffer, FrequencyPeak* peaks) {
    // Convert ADC samples (12-bit unsigned) to float and normalize
    // ADC range: 0-4095, convert to -1.0 to 1.0
    for (int i = 0; i < FFT_SIZE; i++) {
        fft_input[i] = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
    }

    // Perform FFT
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);

    // Calculate magnitude for each frequency bin
    // fft_output contains interleaved real and imaginary components
    float magnitudes[FFT_SIZE / 2];

    // DC component (bin 0)
    magnitudes[0] = fft_output[0];

    // Other frequency bins
    for (int i = 1; i < FFT_SIZE / 2; i++) {
        float real = fft_output[2 * i];
        float imag = fft_output[2 * i + 1];
        magnitudes[i] = arm_sqrt_f32(real * real + imag * imag);
    }

    // Find top frequencies
    findTopFrequencies(magnitudes, peaks);
}

void findTopFrequencies(float* magnitudes, FrequencyPeak* peaks) {
    // Initialize peaks array
    for (int i = 0; i < NUM_FREQUENCIES; i++) {
        peaks[i].frequency = 0.0f;
        peaks[i].magnitude = 0.0f;
    }

    // Find top N peaks
    // Skip DC component (bin 0) as it's not useful for audio
    for (int bin = 1; bin < FFT_SIZE / 2; bin++) {
        float mag = magnitudes[bin];

        // Check if this magnitude is larger than any current peak
        for (int j = 0; j < NUM_FREQUENCIES; j++) {
            if (mag > peaks[j].magnitude) {
                // Shift lower peaks down
                for (int k = NUM_FREQUENCIES - 1; k > j; k--) {
                    peaks[k] = peaks[k - 1];
                }

                // Insert new peak
                peaks[j].magnitude = mag;
                peaks[j].frequency = (float)bin * SAMPLE_RATE / (float)FFT_SIZE;
                break;
            }
        }
    }
}
