# Musical Tesla Coil - STM32 FFT Processing

## Project Overview

This project implements real-time audio frequency analysis for a musical Tesla coil. The STM32L432KC microcontroller samples audio input from a DFPLAYER mini module, performs Fast Fourier Transform (FFT) analysis, and outputs frequency information to an FPGA for LED visualization.

## Hardware Setup

### Required Components
- STM32L432KC Nucleo board
- DFPLAYER mini audio module
- FPGA (iCE40 - already programmed with spectrum analyzer)
- Audio source connected to DFPLAYER

### Pin Connections

#### STM32 Connections
- **PA0** (ADC1_IN5): Audio input from DFPLAYER mini analog output
- **PA6** (TIM16_CH1): Square wave output to FPGA
- **GND**: Common ground with DFPLAYER and FPGA

#### DFPLAYER Mini
Connect the DAC output (analog audio) to PA0 through a voltage divider if needed (DFPLAYER outputs 0-3.3V which is safe for STM32).

## Software Architecture

### Key Features
1. **ADC Sampling**: Continuous audio sampling at 20 kHz using ADC with DMA
2. **FFT Processing**: 512-point Real FFT using ARM CMSIS-DSP library
3. **Peak Detection**: Identifies top 5 dominant frequencies
4. **FPGA Output**: Generates square wave at dominant frequency for FPGA

### File Structure
```
project/stm/
├── lib/
│   ├── STM32L432KC_ADC.c/h      # ADC configuration and control
│   ├── STM32L432KC_DMA.c/h      # DMA for ADC buffering
│   ├── STM32L432KC_RCC.c/h      # Clock configuration (80 MHz PLL)
│   ├── STM32L432KC_GPIO.c/h     # GPIO control
│   ├── STM32L432KC_TIM.c/h      # Timer PWM for output
│   ├── STM32L432KC_FLASH.c/h    # Flash configuration
│   └── fft_processing.c/h       # FFT computation and analysis
├── src/
│   └── main.c                    # Main application
└── README.md                     # This file
```

## Configuration

### ADC Configuration
- **Channel**: ADC1_IN5 (PA0)
- **Resolution**: 12-bit (0-4095)
- **Sample Rate**: 20 kHz (configurable in `fft_processing.h`)
- **DMA**: Circular mode with buffer size 512

### FFT Configuration
- **FFT Size**: 512 points (configurable in `fft_processing.h`)
- **Window**: None (can add Hanning/Hamming for better frequency resolution)
- **Output**: Top 5 frequency peaks with magnitudes

### Clock Configuration
- **System Clock**: 80 MHz (from PLL)
- **ADC Clock**: Synchronous HCLK/1 for maximum speed
- **Timer Clock**: 80 MHz for precise PWM generation

## Building the Project

### Prerequisites
1. **ARM GCC Toolchain**: Install arm-none-eabi-gcc
2. **CMSIS-DSP Library**: Include ARM CMSIS-DSP headers and libraries
3. **SEGGER Embedded Studio** or **STM32CubeIDE**

### Compilation

#### Using SEGGER Embedded Studio
1. Create a new project for STM32L432KC
2. Add all source files from `lib/` and `src/`
3. Include CMSIS-DSP library:
   - Add `libarm_cortexM4lf_math.a` to linker settings
   - Include CMSIS/DSP/Include path
4. Configure preprocessor:
   - Define `ARM_MATH_CM4`
   - Define `__FPU_PRESENT=1`
5. Build and flash to STM32

#### Using Command Line (example Makefile)
See the labs (lab4, lab5) for reference Makefile structure.

## Usage

1. **Power on** the STM32 and DFPLAYER
2. **Play audio** on the DFPLAYER
3. **Observe** the FPGA LEDs displaying frequency bins
4. **Monitor** debug output via UART (optional)

## FPGA Interface

The FPGA expects a square wave input where:
- **Edge frequency** represents the dominant audio frequency
- **100ms window** counts edges to determine frequency bin
- **12 bins** map to different frequency ranges (already implemented in FPGA)

### Frequency Encoding Options

#### Method 1: Direct Square Wave (Simple)
The STM directly toggles GPIO at the dominant frequency. Simple but may have timing jitter.

#### Method 2: Timer PWM (Recommended)
Uses TIM16 to generate precise square wave at the dominant frequency. More accurate and consistent.

## Calibration

### ADC Input Range
- Ensure DFPLAYER output is 0-3.3V
- If different, use voltage divider: `R1/(R1+R2) * Vout = 3.3V max`

### Magnitude Threshold
Adjust the threshold in `outputFrequencyToFPGA()` to filter noise:
```c
if (dominant_freq > 0 && peaks[0].magnitude > 100.0f) {  // Adjust threshold
```

### Sample Rate Adjustment
For different audio ranges, modify `SAMPLE_RATE` in `fft_processing.h`:
- 20 kHz: Captures up to 10 kHz (Nyquist)
- 40 kHz: Captures up to 20 kHz (full audio range)

## Debugging

### UART Output
The code includes `printf()` statements for debugging. Connect a UART terminal to view:
- FFT computation progress
- Top 5 frequencies and magnitudes
- System status

### LED Indicators
Add LED toggle in main loop to verify:
- System is running
- Buffer processing is happening
- DMA interrupts are firing

## Performance Notes

- **FFT Computation Time**: ~5-10ms for 512-point FFT on 80 MHz Cortex-M4
- **Update Rate**: ~20-40 Hz (depends on FFT size and processing)
- **Latency**: ~25-50ms from audio input to FPGA output

## Troubleshooting

### No ADC readings
- Check pin connection to PA0
- Verify ADC clock is enabled
- Check voltage is in range 0-3.3V

### No FFT output
- Verify CMSIS-DSP library is linked
- Check DMA interrupt is enabled
- Monitor buffer_ready flag

### FPGA not responding
- Verify square wave output on PA6 with oscilloscope
- Check frequency range matches FPGA expectations
- Ensure common ground between STM and FPGA

## Future Enhancements

1. **Multi-frequency output**: Encode multiple frequencies using pulse patterns
2. **Windowing**: Add Hanning or Hamming window for better frequency resolution
3. **Dynamic range**: Implement automatic gain control
4. **Beat detection**: Add rhythm detection for visual effects
5. **UART control**: Remote configuration via UART

## References

- STM32L432KC Reference Manual
- ARM CMSIS-DSP Library Documentation
- HMC E155 Course Materials (labs 4, 5, 6)
- FPGA spectrum analyzer code in `project/fpga/`

## Author

Sorin Jayaweera
HMC E155 - Microprocessors Final Project
Fall 2025
