# Modifications Summary - Tesla Coil + DFPlayer Integration

## Overview
This document summarizes all modifications made to enable integration of FFT-based Tesla coil controller and DFPlayer Mini MP3 player on a single STM32L432KC.

## Modified Files

### FFT Project (`stm/fft/`)

#### 1. `src/main.c` - MODIFIED
**Purpose:** Pin remapping to avoid DFPlayer conflicts

**Changes:**
1. **Pin Definitions (lines 50-52)**
   ```c
   // OLD:
   #define LED_PIN         9       // PA9
   #define AUDIO_INPUT_PIN 6       // PA6
   #define ADC_CHANNEL     11      // ADC1 Channel 11

   // NEW:
   #define LED_PIN         0       // PB0 (Board D3)
   #define AUDIO_INPUT_PIN 5       // PA5 (Board A4)
   #define ADC_CHANNEL     10      // ADC1 Channel 10
   ```

2. **GPIO Initialization (lines 292-301)**
   ```c
   // OLD:
   RCC->AHB2ENR |= (1 << 0);  // Enable GPIOA only
   pinMode(LED_PIN, GPIO_OUTPUT);
   pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);

   // NEW:
   RCC->AHB2ENR |= (1 << 0) | (1 << 1);  // Enable GPIOA and GPIOB

   // PB0 as digital output - Direct GPIOB register access
   GPIOB->MODER |= (0b1 << (2 * LED_PIN));
   GPIOB->MODER &= ~(0b1 << (2 * LED_PIN + 1));

   pinMode(AUDIO_INPUT_PIN, GPIO_ANALOG);  // PA5 on GPIOA
   ```

3. **GPIO Write Operations (lines 370-375)**
   ```c
   // OLD:
   digitalWrite(LED_PIN, output_state ? GPIO_HIGH : GPIO_LOW);

   // NEW:
   if (output_state) {
       GPIOB->ODR |= (1 << LED_PIN);   // Set PB0 HIGH
   } else {
       GPIOB->ODR &= ~(1 << LED_PIN);  // Set PB0 LOW
   }
   ```

4. **Documentation Updates**
   - Updated hardware configuration comments (lines 11-17)
   - Updated all function comments referencing PA6/PA9
   - Updated printf statements with new pin names

**Reason:** Library GPIO functions default to GPIOA, but LED_PIN moved to GPIOB (PB0), requiring direct register access.

---

### DFPlayer Project (`stm/dfplayermini/`)

#### 2. `main.c` - MODIFIED
**Purpose:** Change timer from TIM15 to TIM2 to avoid FFT synthesis conflict

**Changes:**
1. **Timer Initialization (lines 31-33)**
   ```c
   // OLD:
   // Enable TIM15 for delays
   RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
   initTIM(TIM15);

   // NEW:
   // Enable TIM2 for delays (TIM15 used by FFT synthesis)
   RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
   initTIM(TIM2);
   ```

2. **Delay Calls (lines 43, 53)**
   ```c
   // OLD:
   delay_millis(TIM15, 100);
   delay_millis(TIM15, 10);

   // NEW:
   delay_millis(TIM2, 100);
   delay_millis(TIM2, 10);
   ```

**Reason:** FFT uses TIM15 for 100 kHz synthesis interrupts; DFPlayer needs different timer for delays.

#### 3. `DFPLAYER_MINI.c` - MODIFIED
**Purpose:** Update all delay calls to use TIM2

**Changes:**
All `delay_millis(TIM15, ...)` calls changed to `delay_millis(TIM2, ...)` in:
- `DF_Init()` - lines 57, 61, 63 (3 calls)
- `DF_PlayFromStart()` - line 68 (1 call)
- `DF_Next()` - line 75 (1 call)
- `DF_Previous()` - line 80 (1 call)
- `DF_Pause()` - line 85 (1 call)
- `DF_Playback()` - line 90 (1 call)
- `DF_SetVolume()` - line 95 (1 call)
- `DF_PlayTrack()` - line 100 (1 call)
- `Check_Key()` - lines 108, 124, 131 (3 calls)

**Total:** 13 delay calls updated from TIM15 to TIM2

**Reason:** Synchronize with main.c timer change.

---

## Final Resource Allocation

### Pin Mapping Table
| Pin  | FFT Usage          | DFPlayer Usage     | Final Assignment |
|------|--------------------|--------------------|------------------|
| PA5  | ADC input (Ch 10)  | -                  | FFT ADC          |
| PA6  | -                  | Pause button       | DFPlayer button  |
| PA8  | -                  | Previous button    | DFPlayer button  |
| PA9  | -                  | USART1 TX          | DFPlayer UART    |
| PA10 | -                  | USART1 RX          | DFPlayer UART    |
| PB0  | Square wave output | -                  | FFT output       |
| PB7  | -                  | Next button        | DFPlayer button  |

### Peripheral Allocation Table
| Peripheral | FFT Usage                      | DFPlayer Usage           | Conflict? |
|------------|--------------------------------|--------------------------|-----------|
| ADC1       | Audio sampling (Ch 10, PA5)    | -                        | ✓ No      |
| USART1     | -                              | DFPlayer serial (9600)   | ✓ No      |
| TIM2       | -                              | Delay functions          | ✓ No      |
| TIM6       | ADC trigger (8 kHz)            | -                        | ✓ No      |
| TIM15      | Synthesis ISR (100 kHz)        | -                        | ✓ No      |
| DMA1 Ch1   | ADC data transfer              | -                        | ✓ No      |
| GPIOA      | PA5 (ADC input)                | PA6, PA8, PA9, PA10      | ✓ No      |
| GPIOB      | PB0 (output)                   | PB7 (button)             | ✓ No      |

✅ **All conflicts resolved!**

## Conflict Resolution Strategy

### Original Conflicts Identified
1. **PA6** - FFT ADC input vs DFPlayer Pause button
2. **PA9** - FFT square wave output vs DFPlayer USART1 TX
3. **TIM15** - FFT synthesis ISR vs DFPlayer delay functions

### Resolution Applied
1. **PA6 conflict** → Moved FFT ADC input to PA5 (Channel 10)
2. **PA9 conflict** → Moved FFT output to PB0
3. **TIM15 conflict** → Moved DFPlayer delays to TIM2

### Why This Approach?
- **FFT pins are flexible** - ADC can use multiple channels, GPIO output can be any pin
- **DFPlayer USART1 is fixed** - PA9/PA10 are hardware UART pins, difficult to remap
- **TIM15 for FFT is preferred** - High-frequency ISR works well on APB2 timer
- **TIM2 for delays is acceptable** - Any basic timer works for blocking delays

## Testing Checklist

### Before Integration
- [ ] Test FFT code with PA5 (ADC) and PB0 (output)
- [ ] Test DFPlayer code with TIM2 delays
- [ ] Verify no compilation errors
- [ ] Verify no pin conflicts remain

### After Integration
- [ ] FFT audio analysis functional
- [ ] DFPlayer button controls functional
- [ ] Both systems run simultaneously
- [ ] No timing interference
- [ ] No resource contention

## Next Steps

1. **Commit Changes** - Save all modifications to git
2. **Test Independently** - Validate each modified project works alone
3. **Create Merged Code** - Combine both into single main.c
4. **Hardware Validation** - Test on STM32L432KC with both peripherals connected

## Notes

- Library files (STM32L432KC_GPIO.c, etc.) were not modified
- Only application code and peripheral usage changed
- All modifications maintain backward compatibility within each project
- Integration requires careful initialization order (see INTEGRATION_PLAN.md)
