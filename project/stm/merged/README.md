# Musical Tesla Coil - Integrated System

This is the merged codebase combining FFT-based frequency analysis with DFPlayer Mini MP3 playback control for a musical Tesla coil project.

## Features

### FFT Audio Analysis
- Samples audio at 8 kHz via ADC (PA5)
- 256-point FFT for frequency detection
- Multi-frequency synthesis (up to 5 simultaneous frequencies)
- Outputs square wave to Tesla coil driver on PB0

### DFPlayer MP3 Control
- Button-controlled music playback
- Previous, Pause/Play, Next track controls
- USART1 communication with DFPlayer Mini at 9600 baud

## Hardware Connections

### Audio Input/Output
- **PA5** (A4) - Audio input for FFT analysis (3V p-p, 1.5V offset)
- **PB0** (D3) - Square wave output to Tesla coil driver

### DFPlayer Mini
- **PA9** (D1) - STM32 TX → DFPlayer RX
- **PA10** (D0) - STM32 RX ← DFPlayer TX
- **VCC/GND** - Power (3.3V-5V)
- **SPK1/SPK2** - Speaker output

### Control Buttons
- **PA8** (D9) - Previous track
- **PA6** (A5) - Pause/Play toggle
- **PB7** (D4) - Next track

## Resource Allocation

| Peripheral | Usage |
|------------|-------|
| ADC1 CH10 | Audio sampling (PA5) |
| USART1 | DFPlayer communication (PA9/PA10) |
| TIM2 | DFPlayer delays |
| TIM6 | ADC trigger (8 kHz) |
| TIM15 | Synthesis engine (100 kHz) |
| DMA1 CH1 | ADC data transfer |

## Pin Summary

| Pin | Function | Board Label |
|-----|----------|-------------|
| PA5 | FFT ADC input | A4 |
| PA6 | Pause button | A5 |
| PA8 | Previous button | D9 |
| PA9 | USART1 TX | D1 |
| PA10 | USART1 RX | D0 |
| PB0 | Tesla coil output | D3 |
| PB7 | Next button | D4 |

## System Operation

1. **Initialization**: System configures all peripherals (ADC, USART, Timers, DMA)
2. **DFPlayer Start**: Begins playing first track from microSD card
3. **FFT Processing**: Continuously analyzes audio input on PA5
4. **Frequency Synthesis**: Generates square waves matching detected frequencies
5. **Button Control**: Polls buttons every 10ms for track control

## Main Loop

```c
while(1) {
    Check_Key(dfplayer_usart);  // Check for button presses

    if (buffer_ready) {
        // FFT processing
        // Frequency detection
        // Update oscillators
    }

    delay_millis(TIM2, 10);  // Rate limiting
}
```

## Configuration

Adjust these parameters in `main.c`:

```c
#define FREQ_THRESHOLD      100.0f   // Min frequency (Hz)
#define MAX_FREQ_THRESHOLD  2000.0f  // Max frequency (Hz)
#define N_FREQ              5         // Number of frequencies to synthesize
```

## Building

This project requires:
- SEGGER Embedded Studio or compatible ARM GCC toolchain
- STM32L432KC target configuration
- Appropriate linker script for STM32L432KC

## Library Dependencies

- `STM32L432KC_RCC.h/c` - Clock configuration
- `STM32L432KC_GPIO.h/c` - GPIO control
- `STM32L432KC_ADC.h/c` - ADC configuration
- `STM32L432KC_DMA.h/c` - DMA setup
- `STM32L432KC_TIM.h/c` - Timer configuration
- `STM32L432KC_USART.h/c` - UART communication
- `STM32L432KC_FLASH.h/c` - Flash timing
- `DFPLAYER_MINI.h/c` - DFPlayer control library

## Notes

- All resource conflicts have been resolved (see `INTEGRATION_PLAN.md`)
- FFT and DFPlayer use separate timers (TIM15 vs TIM2)
- GPIO operations on PB0 use direct register access
- Button inputs should have external pulldown resistors or be connected through switches to GND

## Testing

1. Connect all hardware as specified above
2. Insert microSD card with MP3 files (0001.mp3, 0002.mp3, etc.) into DFPlayer
3. Flash the compiled code to STM32L432KC
4. Feed audio signal into PA5 for FFT analysis
5. Use buttons to control playback
6. Observe square wave output on PB0

## Troubleshooting

**No audio output**: Check DFPlayer connections and microSD card
**No FFT response**: Verify audio input amplitude (3V p-p, 1.5V DC offset)
**Buttons not working**: Check for proper pulldown resistors
**No Tesla output**: Verify PB0 connection and FFT detection threshold

## References

- Original FFT code: `../fft/src/main.c`
- Original DFPlayer code: `../dfplayermini/main.c`
- Integration plan: `../INTEGRATION_PLAN.md`
- Modifications summary: `../MODIFICATIONS_SUMMARY.md`
