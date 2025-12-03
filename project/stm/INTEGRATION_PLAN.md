# Tesla Coil + DFPlayer Integration Plan

## Project Overview
Merge two STM32L432KC projects:
1. **FFT-based Tesla Coil Controller** - Audio frequency analysis with multi-frequency synthesis
2. **DFPlayer Mini MP3 Player** - Button-controlled music playback

## Resource Allocation (Final)

### FFT Resources (MODIFIED)
| Resource | Pin/Peripheral | Function | Board Label |
|----------|---------------|----------|-------------|
| ADC Input | PA5 | Audio input (ADC1 Channel 10) | A4 |
| Output | PB0 | Square wave to Tesla coil | D3 |
| Timer | TIM6 | ADC trigger at 8 kHz | - |
| Timer | TIM15 | Synthesis engine at 100 kHz | - |
| DMA | DMA1 Ch1 | ADC data transfer | - |
| ADC | ADC1 | 12-bit audio sampling | - |

**Changes Made:**
- PA6 → PA5 (ADC input, Channel 11 → 10)
- PA9 → PB0 (Square wave output)

### DFPlayer Resources (UNCHANGED)
| Resource | Pin/Peripheral | Function | Board Label |
|----------|---------------|----------|-------------|
| Previous Button | PA8 | Previous track | D9 |
| Pause Button | PA6 | Pause/Play toggle | A5 |
| Next Button | PB7 | Next track | D4 |
| USART1 TX | PA9 | DFPlayer RX | D1 |
| USART1 RX | PA10 | DFPlayer TX | D0 |
| Timer | TIM15 | Delay functions | - |
| USART | USART1 | 9600 baud serial | - |

### Shared Resources
| Resource | FFT Usage | DFPlayer Usage | Conflict? |
|----------|-----------|----------------|-----------|
| TIM15 | 100 kHz synthesis ISR | delay_millis() blocking calls | ⚠️ POTENTIAL |
| Clock | 80 MHz system clock | 80 MHz system clock | ✓ OK |
| GPIO Ports | GPIOA, GPIOB | GPIOA, GPIOB | ✓ OK |

## Conflicts Identified

### ✅ Resolved Conflicts
1. **PA6** - Originally used by both
   - FFT moved to PA5 ✓
   - DFPlayer keeps PA6 for button ✓

2. **PA9** - Originally used by both
   - FFT moved to PB0 ✓
   - DFPlayer keeps PA9 for USART1 ✓

### ⚠️ Potential Conflict: TIM15
**Issue:** Both projects use TIM15 differently
- FFT: Interrupt-driven at 100 kHz for phase accumulator synthesis
- DFPlayer: Blocking `delay_millis()` function

**Impact:** DFPlayer's delay_millis() will not work while TIM15 is configured for FFT synthesis interrupts.

**Solution Options:**
1. **Option A (Recommended):** Use different timers
   - FFT: Keep TIM15 for synthesis (100 kHz ISR)
   - DFPlayer: Use TIM2 for delay functions

2. **Option B:** Rewrite delay_millis() to use systick or busy-wait loops

3. **Option C:** Disable FFT synthesis during DFPlayer commands (not recommended)

**Chosen Solution:** Option A - Remap DFPlayer to use TIM2 for delays

## Integration Strategy

### Phase 1: Separate Validation ✓
- [x] FFT code modified with new pins (PA5, PB0)
- [x] DFPlayer code reviewed - no changes needed yet
- [ ] Test FFT code independently
- [ ] Test DFPlayer code independently

### Phase 2: Timer Resolution (Current)
- [ ] Modify DFPlayer to use TIM2 instead of TIM15 for delays
- [ ] Create new delay function: `delay_millis_tim2()`
- [ ] Update all DFPlayer delay calls

### Phase 3: Code Merge
- [ ] Combine initialization routines
- [ ] Merge main loops
- [ ] Handle shared peripherals (RCC, GPIO enables)
- [ ] Test combined functionality

### Phase 4: Testing
- [ ] Verify FFT audio analysis works
- [ ] Verify DFPlayer button controls work
- [ ] Test both systems running simultaneously
- [ ] Validate no interference between systems

## File Structure

```
project/stm/
├── fft/
│   ├── src/main.c          (Modified - PA5, PB0)
│   └── lib/                (STM32 peripheral libraries)
├── dfplayermini/
│   ├── main.c              (Original)
│   ├── DFPLAYER_MINI.c     (Root version - PA8, PA6, PB7)
│   └── lib/                (STM32 + DFPlayer libraries)
└── merged/                 (TO BE CREATED)
    ├── main.c              (Combined program)
    └── lib/                (Merged libraries)
```

## Next Steps

1. **Resolve TIM15 conflict** by moving DFPlayer delays to TIM2
2. **Create merged main.c** combining both programs
3. **Test independently** before final integration
4. **Validate combined system** on hardware

## Pin Summary (Quick Reference)

| Pin | FFT | DFPlayer | Final Usage |
|-----|-----|----------|-------------|
| PA5 | ADC input | - | FFT ADC |
| PA6 | - | Pause button | DFPlayer button |
| PA8 | - | Previous button | DFPlayer button |
| PA9 | - | USART1 TX | DFPlayer UART |
| PA10| - | USART1 RX | DFPlayer UART |
| PB0 | Output | - | FFT output |
| PB7 | - | Next button | DFPlayer button |

✓ **All pin conflicts resolved!**
