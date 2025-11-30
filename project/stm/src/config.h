// config.h
// Configuration settings for Musical Tesla Coil project

#ifndef CONFIG_H
#define CONFIG_H

///////////////////////////////////////////////////////////////////////////////
// Hardware Pin Configuration
///////////////////////////////////////////////////////////////////////////////

// ADC Input
#define AUDIO_INPUT_PIN     0       // PA0 - Audio input from DFPLAYER
#define ADC_CHANNEL         5       // ADC1_IN5 (corresponds to PA0)

// Digital Output to FPGA
#define SQUARE_OUT_PIN      6       // PA6 - Square wave to FPGA

///////////////////////////////////////////////////////////////////////////////
// FFT Configuration
///////////////////////////////////////////////////////////////////////////////

// FFT Size - must be power of 2 (128, 256, 512, 1024, etc.)
// Larger FFT = better frequency resolution but slower processing
#define FFT_SIZE            512

// Number of top frequencies to extract
#define NUM_FREQUENCIES     5

///////////////////////////////////////////////////////////////////////////////
// ADC/Sampling Configuration
///////////////////////////////////////////////////////////////////////////////

// Sample rate in Hz
// For audio: 20 kHz captures up to 10 kHz (Nyquist theorem)
// Increase for wider frequency range
#define SAMPLE_RATE         20000

// Buffer size (should match FFT_SIZE)
#define BUFFER_SIZE         FFT_SIZE

///////////////////////////////////////////////////////////////////////////////
// Signal Processing Configuration
///////////////////////////////////////////////////////////////////////////////

// Magnitude threshold to filter noise
// Adjust based on your signal strength
#define MAGNITUDE_THRESHOLD 100.0f

// Number of pulses to send per update
// More pulses = smoother but slower updates
#define PULSES_PER_UPDATE   10

///////////////////////////////////////////////////////////////////////////////
// System Configuration
///////////////////////////////////////////////////////////////////////////////

// System clock frequency (set by PLL configuration)
#define SYSCLK_FREQ         80000000  // 80 MHz

// Enable debug output via UART
#define DEBUG_OUTPUT        1

// Update rate control (ms between FFT updates)
#define UPDATE_PERIOD_MS    50

#endif
