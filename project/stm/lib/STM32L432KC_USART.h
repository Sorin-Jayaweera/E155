// STM32F401RE_USART.h
// Header for USART functions

#ifndef STM32L4_USART_H
#define STM32L4_USART_H

#include <stdint.h>
#include <stm32l432xx.h>  // CMSIS library - provides USART_TypeDef and register bit definitions

// Defines for USART case statements
#define USART1_ID   1
#define USART2_ID   2

// HSI frequency needed for baud rate calculation
#ifndef HSI_FREQ
#define HSI_FREQ 16000000
#endif

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

USART_TypeDef * id2Port(int USART_ID);
USART_TypeDef * initUSART(int USART_ID, int baud_rate);
void sendChar(USART_TypeDef * USART, char data);
char readChar(USART_TypeDef * USART);
void sendString(USART_TypeDef * USART, char * charArray);
void readString(USART_TypeDef * USART, char * charArray);

#endif