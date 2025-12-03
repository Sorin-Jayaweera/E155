// DFPLAYER_MINI.h
// DFPlayer Mini MP3 module control library
// Adapted for STM32L432KC with E155-style programming

#ifndef DFPLAYER_MINI_H
#define DFPLAYER_MINI_H

#include <stdint.h>
#include <stm32l432xx.h>
#include "STM32L432KC.h"

// DFPlayer Mini control functions
void DF_Init(USART_TypeDef * USART, uint8_t volume);
void DF_PlayFromStart(USART_TypeDef * USART);
void DF_Next(USART_TypeDef * USART);
void DF_Previous(USART_TypeDef * USART);
void DF_Pause(USART_TypeDef * USART);
void DF_Playback(USART_TypeDef * USART);
void DF_SetVolume(USART_TypeDef * USART, uint8_t volume);
void DF_PlayTrack(USART_TypeDef * USART, uint8_t track);

// Button check function
void Check_Key(USART_TypeDef * USART);

#endif // DFPLAYER_MINI_H