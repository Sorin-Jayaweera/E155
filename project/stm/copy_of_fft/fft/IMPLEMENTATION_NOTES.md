# Implementation Notes

## Key Design Decisions

### 1. ADC Configuration
- **Channel**: Using ADC1_IN5 on PA0
- **DMA**: Circular mode for continuous sampling
- **Sampling Rate**: 20 kHz (configurable)
  - At 80 MHz system clock with fast ADC sampling (2.5 cycles)
  - Total conversion time: ~2.5 + 12.5 = 15 ADC cycles
  - ADC runs at 80 MHz → 15 cycles = 0.1875 µs per sample
  - Theoretical max: ~5.3 Msamples/s
  - Configured for 20 kHz for audio applications

### 2. FFT Processing
- **Library**: ARM CMSIS-DSP (optimized for Cortex-M4)
- **Size**: 512 points (trade-off between resolution and speed)
- **Type**: Real FFT (RFFT) since input is real-valued audio
- **Frequency Resolution**: SAMPLE_RATE / FFT_SIZE = 20000 / 512 ≈ 39 Hz per bin

### 3. FPGA Communication
The FPGA expects edge counts in a 100ms window. Two methods implemented:

#### Method 1: Direct GPIO Toggle
- Simple but has timing jitter
- Good for testing
- Implementation in `outputFrequencyToFPGA()`

#### Method 2: Timer PWM (Recommended)
- Uses TIM16 for precise frequency generation
- Cleaner signal
- Implementation in `setupTimerForSquareWave()` and `updateSquareWaveFrequency()`

### 4. Memory Usage
```
FFT Input Buffer:   512 * 4 bytes = 2 KB (float)
FFT Output Buffer:  512 * 4 bytes = 2 KB (float)
ADC Buffer:         512 * 2 bytes = 1 KB (uint16_t)
Total:              ~5 KB
```
STM32L432KC has 64 KB RAM, so plenty of headroom.

## CMSIS-DSP Integration

### Required Files
You'll need to add these to your project:

1. **Include Path**: `CMSIS/DSP/Include/`
2. **Library**: `libarm_cortexM4lf_math.a` (for Cortex-M4 with FPU)
3. **Preprocessor Defines**:
   - `ARM_MATH_CM4`
   - `__FPU_PRESENT=1`

### Where to Get CMSIS-DSP
- Download from: https://github.com/ARM-software/CMSIS-DSP
- Or include via STM32CubeL4 package

## Potential Issues and Solutions

### Issue 1: ADC Overrun
**Symptom**: Missed samples, corrupted data
**Solution**:
- Increase DMA priority
- Ensure FFT processing completes before next buffer
- Use double buffering (process one buffer while filling the other)

### Issue 2: FFT Processing Too Slow
**Symptom**: Can't keep up with sample rate
**Solution**:
- Reduce FFT_SIZE to 256
- Optimize interrupt handlers
- Process FFT less frequently (skip some buffers)

### Issue 3: Noisy Frequency Detection
**Symptom**: Jumping between frequencies
**Solution**:
- Increase MAGNITUDE_THRESHOLD
- Add moving average filter
- Implement peak tracking (hysteresis)
- Add windowing function (Hanning, Hamming)

### Issue 4: FPGA Not Detecting Frequencies
**Symptom**: No LED response
**Solution**:
- Verify square wave with oscilloscope
- Check frequency range matches FPGA bins
- Ensure sufficient magnitude (volume)
- Verify common ground connection

## Optimization Opportunities

### 1. Double Buffering
Implement true double buffering for zero-latency:
```c
uint16_t buffer_a[FFT_SIZE];
uint16_t buffer_b[FFT_SIZE];
volatile uint16_t* active_buffer = buffer_a;
volatile uint16_t* process_buffer = buffer_b;
```

### 2. Frequency Smoothing
Add exponential moving average:
```c
float alpha = 0.3;  // Smoothing factor
smoothed_freq = alpha * new_freq + (1 - alpha) * smoothed_freq;
```

### 3. Windowing
Reduce spectral leakage:
```c
// Hanning window
for (int i = 0; i < FFT_SIZE; i++) {
    float window = 0.5 * (1 - cosf(2 * PI * i / FFT_SIZE));
    fft_input[i] *= window;
}
```

### 4. Compile Optimizations
- Enable `-O3` optimization
- Use `-ffast-math` for faster floating point
- Enable FPU: `-mfpu=fpv4-sp-d16 -mfloat-abi=hard`

## Testing Checklist

- [ ] Verify ADC samples clean audio signal
- [ ] Confirm DMA interrupt fires regularly
- [ ] Check FFT output shows expected frequencies
- [ ] Validate square wave output frequency
- [ ] Test with various audio sources (sine wave, music, voice)
- [ ] Measure performance (FFT computation time)
- [ ] Verify FPGA responds correctly
- [ ] Test edge cases (silence, very loud, white noise)

## Performance Measurements

Estimate timing (to be measured):
- ADC DMA fill (512 samples @ 20 kHz): 25.6 ms
- FFT computation (512-point): ~5-10 ms
- Peak finding: <1 ms
- Total latency: ~30-40 ms
- Update rate: ~25-30 Hz

## Next Steps

1. **Build and flash** to STM32
2. **Test ADC** with known frequency source
3. **Verify FFT** output with pure tones
4. **Integrate with FPGA** and test complete system
5. **Optimize** based on performance measurements
6. **Add enhancements** (windowing, smoothing, multi-frequency output)

## Reference Documents

- [STM32L432KC Datasheet](https://www.st.com/resource/en/datasheet/stm32l432kc.pdf)
- [STM32L4 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00151940.pdf)
- [ARM CMSIS-DSP Documentation](https://arm-software.github.io/CMSIS_5/DSP/html/index.html)
- [FFT Tutorial](https://en.wikipedia.org/wiki/Fast_Fourier_transform)
