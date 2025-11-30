// fft_processing.c
// Simplified standalone FFT implementation

#include "fft_processing.h"
#include <math.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Math constants
///////////////////////////////////////////////////////////////////////////////

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

///////////////////////////////////////////////////////////////////////////////
// Private structures
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    float real;
    float imag;
} Complex;

///////////////////////////////////////////////////////////////////////////////
// Private variables
///////////////////////////////////////////////////////////////////////////////

static Complex fft_buffer[FFT_SIZE];
static float magnitudes[FFT_SIZE / 2];

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////

// Simple FFT implementation (Cooley-Tukey radix-2)
void fft_compute(Complex* data, int n) {
    // Bit-reversal permutation
    int i, j, k;
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

    // Cooley-Tukey FFT
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

void findTopFrequencies(FrequencyPeak* peaks) {
    // Initialize peaks array
    for (int i = 0; i < NUM_FREQUENCIES; i++) {
        peaks[i].frequency = 0.0f;
        peaks[i].magnitude = 0.0f;
    }

    // Find top N peaks (skip DC bin 0)
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

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////

void initFFT(void) {
    // Nothing to initialize for standalone version
}

void processFFT(uint16_t* adc_buffer, FrequencyPeak* peaks) {
    // Convert ADC samples to complex (real part only, normalize to -1 to 1)
    for (int i = 0; i < FFT_SIZE; i++) {
        fft_buffer[i].real = ((float)adc_buffer[i] - 2048.0f) / 2048.0f;
        fft_buffer[i].imag = 0.0f;
    }

    // Perform FFT
    fft_compute(fft_buffer, FFT_SIZE);

    // Calculate magnitudes
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float real = fft_buffer[i].real;
        float imag = fft_buffer[i].imag;
        magnitudes[i] = sqrtf(real * real + imag * imag);
    }

    // Find top frequencies
    findTopFrequencies(peaks);
}
