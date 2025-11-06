//Sorin Jayaweera
//10/4/2025
#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom defines
///////////////////////////////////////////////////////////////////////////////

#define quadencA PA1
#define quadencB PA2
#define DELAY_TIM TIM2



void setupChip(void);
void initializeTimer(void);
void configureInterrupts(void);
#endif // MAIN_H