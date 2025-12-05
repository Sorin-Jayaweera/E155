# Musical Tesla Coil - Comprehensive Analytical Performance Report

## Executive Summary

This report provides detailed analytical measures for both the FPGA spectrum analyzer and MCU FFT-based Tesla coil driver, including frequency accuracy, resolution, latency, and quality metrics.

---

## 1. FPGA SPECTRUM ANALYZER

### 1.1 Architecture
- **Clock**: 48 MHz internal oscillator
- **Measurement Window**: 100 ms (4,800,000 clock cycles)
- **Method**: Falling edge counting of input square wave
- **Display**: 12 frequency buckets with PWM brightness control
- **Update Rate**: 10 Hz (100 ms window)

### 1.2 Frequency Bucket Mapping

The FPGA counts falling edges over 100 ms and maps to 12 buckets:

| Bucket | Edge Count Range | Frequency Range (Hz) | Bucket Width | Center Frequency |
|--------|------------------|---------------------|--------------|------------------|
| 0      | 0 - 16           | 0 - 160             | 160 Hz       | 80 Hz            |
| 1      | 17 - 33          | 170 - 330           | 160 Hz       | 250 Hz           |
| 2      | 34 - 50          | 340 - 500           | 160 Hz       | 420 Hz           |
| 3      | 51 - 67          | 510 - 670           | 160 Hz       | 590 Hz           |
| 4      | 68 - 83          | 680 - 830           | 150 Hz       | 755 Hz           |
| 5      | 84 - 100         | 840 - 1000          | 160 Hz       | 920 Hz           |
| 6      | 101 - 117        | 1010 - 1170         | 160 Hz       | 1090 Hz          |
| 7      | 118 - 134        | 1180 - 1340         | 160 Hz       | 1260 Hz          |
| 8      | 135 - 150        | 1350 - 1500         | 150 Hz       | 1425 Hz          |
| 9      | 151 - 167        | 1510 - 1670         | 160 Hz       | 1590 Hz          |
| 10     | 168 - 184        | 1680 - 1840         | 160 Hz       | 1760 Hz          |
| 11     | > 184            | > 1840              | Unbounded    | N/A              |

**Calculation**: Frequency (Hz) = Edge Count × 10 (since 1 edge/100ms = 10 Hz)

### 1.3 FPGA Frequency Accuracy

#### Absolute Accuracy:
- **Quantization**: ±1 edge count
- **Absolute Error**: ±10 Hz (±1 edge × 10 Hz/edge)
- **Constant error across all frequencies**

#### Relative Accuracy:
- **At 100 Hz** (10 edges): ±10 Hz = **±10.0%** error
- **At 500 Hz** (50 edges): ±10 Hz = **±2.0%** error
- **At 1000 Hz** (100 edges): ±10 Hz = **±1.0%** error
- **At 2000 Hz** (200 edges): ±10 Hz = **±0.5%** error

#### Musical Note Accuracy (A440 Reference):
For A440 (440 Hz, expected 44 edges):
- Measured: 44 edges → 440 Hz ✓ (exact)
- Range: 43-45 edges → 430-450 Hz
- **Accuracy: ±2.3%** (±10 Hz on 440 Hz)

For Middle C (261.6 Hz, expected 26.16 edges):
- Measured: 26 edges → 260 Hz (error: -1.6 Hz)
- Range: 26-27 edges → 260-270 Hz
- **Accuracy: ±3.8%** (±10 Hz on 261.6 Hz)

#### Bucket Resolution:
- **Effective Resolution**: 10 Hz (limited by edge count quantization)
- **Bucket Width**: ~150-160 Hz (cannot distinguish notes within same bucket)
- **Musical Limitation**: Cannot distinguish adjacent semitones (≈6% frequency difference)

### 1.4 FPGA Timing Characteristics

| Parameter | Value | Notes |
|-----------|-------|-------|
| Measurement Latency | 100 ms | Fixed window duration |
| Display Update Rate | 10 Hz | One measurement per window |
| Edge Detection Delay | ~20.8 ns | 1 clock cycle @ 48 MHz |
| PWM Frequency | 20 kHz | 2400 cycles @ 48 MHz (imperceptible) |
| Fade Time | 2 seconds | 256 brightness steps × 7.8 ms |

---

## 2. MCU FFT-BASED SYNTHESIS

### 2.1 Architecture
- **MCU**: STM32L432KC (ARM Cortex-M4F @ 80 MHz)
- **ADC Sample Rate**: 8000 Hz (Nyquist limit: 4000 Hz)
- **FFT Size**: 256 samples (Cooley-Tukey radix-2)
- **Synthesis Rate**: 100 kHz (TIM15 interrupt)
- **Output**: Multi-frequency phase accumulator synthesis

### 2.2 FFT Frequency Resolution

#### Bin Characteristics:
- **Bin Spacing**: 8000 Hz / 256 = **31.25 Hz per bin**
- **Usable Range**: Bin 1 to Bin 127 (31.25 Hz to 3968.75 Hz)
- **DC Bin**: Bin 0 (ignored)
- **Nyquist Limit**: 4000 Hz

#### Musical Note Resolution:

| Note | Frequency (Hz) | Nearest FFT Bin | Bin Frequency (Hz) | Error (Hz) | Error (%) |
|------|----------------|-----------------|-------------------|-----------|-----------|
| C4 (Middle C) | 261.63 | Bin 8 | 250.00 | -11.63 | -4.4% |
| E4 | 329.63 | Bin 11 | 343.75 | +14.12 | +4.3% |
| A4 | 440.00 | Bin 14 | 437.50 | -2.50 | -0.6% |
| C5 | 523.25 | Bin 17 | 531.25 | +8.00 | +1.5% |
| A5 | 880.00 | Bin 28 | 875.00 | -5.00 | -0.6% |

**Observation**: FFT bin quantization causes **±1-4%** error for musical notes due to 31.25 Hz bin spacing.

### 2.3 ADC Sampling Accuracy

#### TIM6 Trigger Timing:
```
System Clock: 80,000,000 Hz
Prescaler: 79 (divides by 80)
Auto-Reload: 124 (counts 0-124 = 125 counts)
Timer Frequency = 80,000,000 / 80 / 125 = 8000.000 Hz (exact)
```

- **Sample Rate Error**: **0.000%** (mathematically exact from integer division)
- **Jitter**: < 12.5 ns (1 clock cycle @ 80 MHz)
- **ADC Resolution**: 12-bit (4096 levels, 0.81 mV @ 3.3V)

#### ADC Accuracy Limitations:
- **Quantization**: ±0.5 LSB = ±0.4 mV
- **INL/DNL**: ±2 LSB typical (±1.6 mV)
- **THD**: -72 dB (STM32L4 datasheet)

### 2.4 Synthesis Output Accuracy

#### TIM15 Synthesis Frequency Accuracy:

The MCU uses phase accumulator synthesis at 100 kHz update rate:

```c
oscillators[i].phase += oscillators[i].frequency / SYNTHESIS_RATE;
if (oscillators[i].phase >= 1.0f) {
    oscillators[i].phase -= 1.0f;
}
output = (oscillators[i].phase < 0.5f);  // 50% duty cycle
```

**TIM15 Timer Accuracy**:
```
System Clock: 80,000,000 Hz
Prescaler: 0 (no division)
Auto-Reload: 799 (counts 0-799 = 800 counts)
TIM15 Frequency = 80,000,000 / 800 = 100,000.000 Hz (exact)
```

**Phase Accumulator Precision**:
- **Data Type**: 32-bit IEEE 754 float
- **Mantissa**: 23 bits + 1 implicit = 24 bits effective
- **Phase Resolution**: 2^-24 ≈ 5.96 × 10^-8 (in range [0, 1))
- **Frequency Resolution**: 100,000 Hz × 5.96 × 10^-8 ≈ **0.006 Hz**

**Example: Synthesizing A440 (440 Hz)**:
```
Phase increment = 440 / 100000 = 0.00440
Float representation: exact (well within 24-bit mantissa precision)
Output frequency: 440.000 Hz (limited only by HSI clock accuracy)
```

#### Clock Source Accuracy (Dominant Error):
- **HSI (Internal RC Oscillator)**: ±1% typical, ±3% max at 25°C
- **After PLL**: Same ±1-3% error propagates to output frequency
- **Temperature drift**: ±1.5% over -40°C to 85°C

**Synthesized Note Accuracy**:
| Target (Hz) | Ideal Phase Increment | Float Error (Hz) | HSI ±1% Error (Hz) | Total Error |
|-------------|----------------------|------------------|-------------------|-------------|
| 100.0       | 0.00100              | < 0.001          | ±1.0              | **±1.0%**   |
| 440.0       | 0.00440              | < 0.001          | ±4.4              | **±1.0%**   |
| 1000.0      | 0.01000              | < 0.001          | ±10.0             | **±1.0%**   |
| 2000.0      | 0.02000              | < 0.001          | ±20.0             | **±1.0%**   |

**Conclusion**: Output frequency accuracy is **dominated by HSI clock accuracy (±1%)**, not by synthesis algorithm precision (±0.001%).

### 2.5 FFT Processing Latency

#### Pipeline Timing:

1. **ADC Sampling Window**: 256 samples / 8000 Hz = **32.0 ms**
2. **DMA Transfer**: Negligible (< 1 µs)
3. **FFT Computation**:
   - Cooley-Tukey radix-2: O(N log₂ N) = 256 × 8 = 2048 complex operations
   - Estimated CPU time: ~500 µs @ 80 MHz with FPU
4. **Peak Finding**: ~50 µs (linear search through 128 bins)
5. **Oscillator Update**: < 10 µs

**Total Latency**: **32.0 ms + 0.56 ms ≈ 32.6 ms**

**Practical Latency**:
- Minimum: 32.6 ms (best case, signal present at start of window)
- Maximum: 64.6 ms (worst case, signal arrives just after window starts)
- **Average: ~48 ms** (1.5 windows)

**Musical Perception**:
- Human pitch perception delay: ~30-50 ms
- **System latency (48 ms) is at the threshold of human perception**

### 2.6 Multi-Frequency Synthesis Quality

The system can synthesize up to 5 simultaneous frequencies (N_FREQ = 5):

**Harmonic Distortion**:
- Each oscillator outputs pure 50% duty cycle square wave
- Square wave contains odd harmonics: f, 3f, 5f, 7f...
- Fundamental power: 81% (8/π² ≈ 0.81)
- 3rd harmonic: 9% relative to fundamental
- **THD ≈ 43%** (inherent to square wave, not system error)

**Multi-tone Distortion**:
- Oscillators are OR'ed together (boolean sum)
- No analog mixing distortion (digital synthesis)
- **Intermodulation**: None (digital domain)

---

## 3. COMPARATIVE ANALYSIS

### 3.1 Frequency Accuracy Comparison

| System | Method | Resolution | Absolute Accuracy | Relative Accuracy @ 440 Hz |
|--------|--------|-----------|-------------------|---------------------------|
| **FPGA** | Edge counting | 10 Hz | ±10 Hz | ±2.3% |
| **MCU FFT** | FFT bin | 31.25 Hz | ±15.6 Hz | ±3.5% |
| **MCU Output** | Phase accumulator | 0.006 Hz | ±4.4 Hz (HSI limited) | ±1.0% |

**Key Insight**: MCU synthesis output is **more accurate (±1%)** than both FPGA measurement (±2.3%) and FFT detection (±3.5%).

### 3.2 Latency Comparison

| System | Measurement Window | Update Rate | Latency |
|--------|-------------------|-------------|---------|
| **FPGA** | 100 ms | 10 Hz | 100 ms |
| **MCU FFT** | 32 ms | 31.25 Hz | ~48 ms average |

**MCU FFT is 2.1× faster** than FPGA spectrum analyzer.

### 3.3 Musical Performance Metrics

#### Note Detection Range:
- **FPGA**: Optimized for 0-2000 Hz (12 buckets covering musical range)
- **MCU**: Configured for 100-2000 Hz (usable FFT range with thresholds)

#### Pitch Discrimination:
- **FPGA**: 160 Hz buckets → **Cannot distinguish semitones** (≈6% spacing)
- **MCU**: 31.25 Hz bins → **Can distinguish whole tones** (≈12% spacing)
- **Human Perception**: ~0.5% pitch difference (sensitive to <1 semitone)

**Neither system can match human pitch discrimination.**

---

## 4. ADDITIONAL ANALYTICAL MEASURES

### 4.1 Dynamic Range

**FPGA**:
- **Input**: Digital square wave (rail-to-rail)
- **Dynamic Range**: N/A (binary input)

**MCU**:
- **ADC**: 12-bit resolution
- **Dynamic Range**: 20 × log₁₀(2¹²) = **72 dB**
- **Effective Range**: ~60 dB (limited by ADC noise floor)

### 4.2 Frequency Range Coverage

**FPGA**:
- Minimum detectable: 10 Hz (1 edge in 100 ms)
- Maximum tracked: > 2000 Hz (200+ edges)
- **Range**: 10 Hz - unlimited

**MCU**:
- Minimum: 31.25 Hz (Bin 1)
- Maximum: 4000 Hz (Nyquist limit)
- Configured range: 100 - 2000 Hz (FREQ_THRESHOLD filtering)
- **Musical Range**: C2 (65 Hz) to B6 (1976 Hz) ≈ 5 octaves

### 4.3 Temporal Resolution

**FPGA**:
- Update rate: 10 Hz
- **Temporal resolution**: 100 ms (cannot detect changes faster than this)

**MCU**:
- Update rate: 31.25 Hz
- **Temporal resolution**: 32 ms (can track faster frequency changes)
- **Synthesis update**: 100 kHz (smooth output transitions)

### 4.4 Signal-to-Noise Ratio (SNR)

**FPGA**:
- Input: Digital (no noise in frequency domain)
- **Noise floor**: Limited by edge detection jitter (~20 ns)
- **SNR**: > 90 dB (digital system)

**MCU FFT**:
- Input: 12-bit ADC (inherent quantization noise)
- FFT bins: 256-point averaging effect
- **SNR**: ~60 dB (ADC-limited)
- **Noise floor**: MAG_THRESHOLD = 10.0 (configured to reject noise)

### 4.5 Power Spectral Density Resolution

**MCU FFT**:
- Bin width: 31.25 Hz
- Window: Rectangular (256 samples)
- **Frequency selectivity**: -13 dB @ ±31.25 Hz (first sidelobe)
- **3 dB bandwidth**: ~15.6 Hz per bin

**Implication**: Closely spaced tones (< 31.25 Hz apart) will leak into adjacent bins.

### 4.6 Maximum Detectable Frequency

**FPGA**:
- Theoretical limit: 24 MHz (half of sampling clock)
- Practical limit: ~10 kHz (edge detector stability)
- **Counter overflow**: 255 edges @ 100 ms = 2550 Hz (8-bit counter)
- **Design limit**: No upper bound (bucket 11 catches all > 1840 Hz)

**MCU**:
- **Nyquist limit**: 4000 Hz (strict)
- Aliasing occurs above 4000 Hz
- Anti-alias filtering: None (relies on input signal bandwidth)

---

## 5. SYSTEM QUALITY METRICS SUMMARY

### 5.1 Recommended Quality Metrics

For future testing and characterization:

#### **1. Frequency Accuracy**
- Test tones: 100, 200, 440, 880, 1000, 1500, 2000 Hz
- Measure: Actual output frequency vs. input
- **Target**: < 2% error for musical notes

#### **2. Frequency Resolution**
- Sweep frequency in 10 Hz steps
- Determine minimum resolvable frequency difference
- **Current**:
  - FPGA: 10 Hz (edge counting quantization)
  - MCU: 31.25 Hz (FFT bin spacing)

#### **3. Latency (Input to Output)**
- Measure time from signal start to output change
- **Current**:
  - FPGA: 100 ms (fixed window)
  - MCU: 32-64 ms (windowed averaging)

#### **4. Update Rate**
- Maximum rate at which system responds to frequency changes
- **Current**:
  - FPGA: 10 Hz
  - MCU: 31.25 Hz

#### **5. Multi-Tone Separation**
- Input two tones (e.g., 440 Hz + 554 Hz = Major 3rd)
- Measure if both are detected correctly
- **Current MCU capability**: Can detect if > 31.25 Hz apart

#### **6. Magnitude Dynamic Range**
- Vary input amplitude from noise floor to saturation
- Measure detection threshold and maximum input
- **MCU**: 60 dB (12-bit ADC effective range)

#### **7. Harmonic Rejection**
- Input 440 Hz fundamental
- Measure if system detects 1320 Hz (3rd harmonic)
- **Current**: RELATIVE_MAG_THRESHOLD = 0.5 (rejects harmonics < 50% of fundamental)

#### **8. Frequency Sweep Response**
- Linear frequency sweep (chirp) 100-2000 Hz over 2 seconds
- Measure tracking accuracy
- **Expected**: Stepwise following with 32 ms latency (MCU)

---

## 6. LIMITATIONS AND IMPROVEMENT OPPORTUNITIES

### 6.1 Current Limitations

**FPGA**:
1. Coarse frequency resolution (10 Hz) - cannot distinguish musical semitones
2. Wide buckets (160 Hz) - poor frequency selectivity
3. Slow update rate (10 Hz / 100 ms latency)

**MCU**:
1. FFT bin quantization (31.25 Hz) - causes ±1-4% note error
2. Limited to 5 simultaneous tones
3. HSI clock drift (±1%) affects output accuracy
4. Square wave output (43% THD inherent)

### 6.2 Potential Improvements

**For Better Frequency Accuracy (<0.5% target)**:

1. **FPGA**: Reduce measurement window to 10 ms for 1 Hz resolution
   - Tradeoff: Higher noise susceptibility

2. **MCU**: Implement parabolic interpolation on FFT bins
   - Can achieve ~0.1 bin accuracy (3.1 Hz instead of 31.25 Hz)
   - Algorithm: Fit parabola to peak bin and neighbors

3. **MCU**: Use external crystal oscillator instead of HSI
   - Accuracy: ±50 ppm (±0.005%) vs. ±1%
   - Cost: +$0.50 and external components

4. **MCU**: Increase FFT size to 512 or 1024 samples
   - Resolution: 15.6 Hz or 7.8 Hz per bin
   - Tradeoff: 2-4× longer latency and computation time

**For Lower Latency (<20 ms target)**:

1. **FPGA**: Use 10 ms window → 10 Hz update rate, but 10× worse frequency resolution
2. **MCU**: Decrease FFT size to 128 samples → 16 ms window, but 62.5 Hz bin spacing

**For Better Musical Performance**:

1. Implement pitch tracking algorithm (e.g., autocorrelation, YIN algorithm)
   - Can achieve <1% pitch accuracy independent of FFT bins
2. Add vibrato detection (frequency modulation < 10 Hz)
3. Implement note quantization to equal temperament scale

---

## 7. CONCLUSION

### Performance Summary:

| Metric | FPGA Value | MCU Value | Best System |
|--------|-----------|-----------|-------------|
| **Frequency Accuracy** | ±2.3% @ 440 Hz | ±1.0% @ 440 Hz | **MCU** |
| **Frequency Resolution** | 10 Hz | 31.25 Hz | **FPGA** |
| **Latency** | 100 ms | ~48 ms | **MCU** |
| **Update Rate** | 10 Hz | 31.25 Hz | **MCU** |
| **Musical Range** | 0-2000+ Hz | 100-2000 Hz | Comparable |

### Key Findings:

1. **MCU synthesis output accuracy (±1%) is primarily limited by HSI clock stability, not algorithm precision (±0.001%)**

2. **FPGA provides better frequency resolution (10 Hz) but at the cost of longer latency (100 ms)**

3. **Neither system can distinguish musical semitones reliably** (semitone ≈ 6% frequency difference, systems have 1-4% error + coarse quantization)

4. **FFT latency (48 ms average) is near the threshold of human perception (30-50 ms)**

5. **Multi-frequency synthesis is limited to 5 simultaneous tones** but has excellent precision (0.006 Hz resolution)

### Recommended Metrics for Future Work:

1. **Frequency tracking accuracy**: < 0.5% (requires interpolation or external crystal)
2. **Latency**: < 30 ms (requires smaller FFT or different algorithm)
3. **Frequency resolution**: < 5 Hz (to distinguish whole musical tones)
4. **Dynamic range**: > 60 dB (current ADC-limited performance)
5. **Musical accuracy**: Quantize to 12-TET scale with ±10 cents accuracy

---

**Report Generated**: 2024-12-05
**System Version**: Musical Tesla Coil Integration (FPGA + MCU)
