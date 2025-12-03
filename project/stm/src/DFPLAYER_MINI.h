// DFPLAYER_MINI.h
// DFPlayer Mini MP3 module control library
// Uses hardware USART1 - ORIGINAL WORKING VERSION

#ifndef DFPLAYER_MINI_H
#define DFPLAYER_MINI_H

#include <stdint.h>

// Forward declarations to avoid header conflicts
// (USART library includes stm32l432xx.h which conflicts with custom E155 headers)
struct USART_TypeDef;
typedef struct USART_TypeDef USART_TypeDef;

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
