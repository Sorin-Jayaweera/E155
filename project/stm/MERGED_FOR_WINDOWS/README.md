# Merged Tesla Coil + DFPlayer Integration Files

## Overview
These files combine the working FFT Tesla coil and DFPlayer Mini projects using the DFPlayer CMSIS-compatible library base.

## What's in This Directory

### Modified Files (USE THESE):
1. **main.c** - Merged main program with both FFT and DFPlayer functionality
2. **STM32L432KC_TIM.h** - Timer library header (DFPlayer base + FFT TIM6/TIM15 functions)
3. **STM32L432KC_TIM.c** - Timer library implementation (merged)

### Library Files to Use from DFPlayer Project:
Copy these from `stm/dfplayermini/` to your `lib/` folder:
- `STM32L432KC_GPIO.c` and `.h` (CMSIS-compatible, sophisticated pin mapping)
- `STM32L432KC_RCC.c` and `.h` (CMSIS-compatible)
- `STM32L432KC_FLASH.c` and `.h` (CMSIS-compatible)
- `STM32L432KC_USART.c` and `.h` (needed for DFPlayer)
- `DFPLAYER_MINI.c` and `.h` (DFPlayer control functions)

### Library Files to Use from FFT Project:
Copy these from `stm/fft/lib/` to your `lib/` folder:
- `STM32L432KC_ADC.c` and `.h` (DFPlayer doesn't have ADC support)
- `STM32L432KC_DMA.c` and `.h` (DFPlayer doesn't have DMA support)

## Installation Steps

### For Your Windows Copy (`C:\Users\sojayaweera\E155\project\stm\copy_of_fft\fft`):

1. **Replace main.c**:
   ```
   Copy: MERGED_FOR_WINDOWS/main.c
   To:   copy_of_fft/fft/src/main.c
   ```

2. **Update TIM library**:
   ```
   Copy: MERGED_FOR_WINDOWS/STM32L432KC_TIM.h
   To:   copy_of_fft/fft/lib/STM32L432KC_TIM.h

   Copy: MERGED_FOR_WINDOWS/STM32L432KC_TIM.c
   To:   copy_of_fft/fft/lib/STM32L432KC_TIM.c
   ```

3. **Use DFPlayer GPIO library** (KEEP THESE, DELETE FFT versions):
   ```
   KEEP: lib/STM32L432KC_GPIO.c/.h (from DFPlayer)
   DELETE: Any GPIO files from FFT project
   ```

4. **Use DFPlayer RCC library** (KEEP THESE, DELETE FFT versions):
   ```
   KEEP: lib/STM32L432KC_RCC.c/.h (from DFPlayer)
   DELETE: Any RCC files from FFT project
   ```

5. **Use DFPlayer FLASH library** (KEEP THESE, DELETE FFT versions):
   ```
   KEEP: lib/STM32L432KC_FLASH.c/.h (from DFPlayer)
   DELETE: Any FLASH files from FFT project
   ```

6. **Keep FFT ADC and DMA libraries**:
   ```
   KEEP: lib/STM32L432KC_ADC.c/.h (from FFT)
   KEEP: lib/STM32L432KC_DMA.c/.h (from FFT)
   ```

7. **Add DFPlayer files**:
   ```
   ADD: lib/STM32L432KC_USART.c/.h (from DFPlayer)
   ADD: lib/DFPLAYER_MINI.c/.h (from DFPlayer)
   ```

## Key Differences from Original FFT Code

### Pin Mapping:
The DFPlayer GPIO library uses sophisticated pin encoding:
- **PA0-PA15** = 0-15
- **PB0-PB15** = 16-31
- **PC0-PC15** = 32-47

However, the merged `main.c` handles this correctly:
- Uses `AUDIO_INPUT_PIN = PA5` with `pinMode()` (DFPlayer GPIO automatically decodes PA5=5 to Port A, Pin 5)
- Uses `LED_PIN = 0` with direct GPIOB register access (for speed in TIM15 ISR)

### CMSIS Compatibility:
All DFPlayer libraries use CMSIS headers (`#include <stm32l432xx.h>`), so they work perfectly with SEGGER Embedded Studio without conflicts.

### ADC_COMMON Naming:
The merged main uses `ADC123_COMMON` (correct CMSIS name for STM32L4) instead of `ADC_COMMON`.

## Hardware Configuration

### FFT System:
- **Audio Input**: PA5 (Board A4, ADC Channel 10)
- **Tesla Output**: PB0 (Board D3)

### DFPlayer System:
- **USART1 TX**: PA9 (Board D1) → DFPlayer RX
- **USART1 RX**: PA10 (Board D0) → DFPlayer TX
- **Button Previous**: PA8 (Board D9)
- **Button Pause/Play**: PA6 (Board A5)
- **Button Next**: PB7 (Board D4)

### Timers:
- **TIM2**: DFPlayer delay functions
- **TIM6**: ADC trigger @ 8 kHz
- **TIM15**: FFT synthesis @ 100 kHz

## Testing

1. Build the project in SEGGER Embedded Studio
2. Connect audio input to PA5 (e.g., from DFPlayer's audio output or external source)
3. Connect Tesla coil driver to PB0
4. Connect DFPlayer buttons as specified above
5. Insert microSD card with MP3 files (001.mp3, 002.mp3, etc.)
6. Flash and run

## Expected Behavior

- MP3 music plays from DFPlayer
- Buttons control playback (previous/pause/next)
- FFT analyzes audio input
- Tesla coil outputs square waves matching detected frequencies
- Serial output shows detected frequencies

## Troubleshooting

### If build fails:
1. Ensure only ONE version of each library file (use DFPlayer base as specified)
2. Check that `stm32l432xx.h` is included in SEGGER's CMSIS package
3. Verify all file paths in project settings

### If FFT doesn't work:
1. Check PA5 has audio signal (measure with oscilloscope)
2. Verify ADC trigger rate is 8 kHz (check TIM6 configuration)
3. Ensure DMA interrupt is firing (add debug printf in DMA ISR)

### If DFPlayer doesn't work:
1. Check USART connections (TX→RX, RX→TX)
2. Verify 9600 baud rate
3. Ensure microSD card is formatted FAT32 with MP3 files in root

## Architecture Notes

This integration uses the **DFPlayer library base** because:
1. DFPlayer libraries are already CMSIS-compatible
2. No type redefinitions with SEGGER's auto-included CMSIS headers
3. More modern and maintainable code structure
4. Better port abstraction (supports pins across all GPIO ports)

The FFT code was adapted to work with this base by:
1. Using DFPlayer GPIO's smart `pinMode()` function
2. Changing ADC_COMMON to ADC123_COMMON
3. Adding TIM6 and TIM15 initialization to the TIM library
4. Merging FFT and DFPlayer main loops

## Future Enhancements

- Lower frequency support (currently 100 Hz minimum, could go to 40 Hz)
- Adjustable synthesis parameters via buttons
- Real-time volume control affecting Tesla coil intensity
- MIDI input support
