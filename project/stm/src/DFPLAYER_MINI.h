// DFPLAYER_MINI.h
// DFPlayer Mini MP3 module control library
// Adapted for STM32L432KC with software UART

#ifndef DFPLAYER_MINI_H
#define DFPLAYER_MINI_H

#include <stdint.h>
#include "../lib/STM32L432KC_GPIO.h"

// DFPlayer Mini control functions
void DF_Init(uint8_t volume);
void DF_PlayFromStart(void);
void DF_Next(void);
void DF_Previous(void);
void DF_Pause(void);
void DF_Playback(void);
void DF_SetVolume(uint8_t volume);
void DF_PlayTrack(uint8_t track);

// Button check function
void Check_Key(void);

#endif // DFPLAYER_MINI_H
