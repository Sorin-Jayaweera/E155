// STM32L432KC_RCC.c
// Source code for RCC functions

#include "STM32L432KC_RCC.h"

void configurePLL() {
    // Set clock to 80 MHz
    // Output freq = (src_clk) * (N/M) / R
    // (4 MHz) * (N/M) / R = 80 MHz
    // M: XX, N: XX, R: XX
    // Use MSI as PLLSRC

    // TODO: Turn off PLL
    
    // TODO: Wait till PLL is unlocked (e.g., off)
    

    // Load configuration
    // TODO: Set PLL SRC to MSI


    // TODO: Set PLLN


    // TODO: Set PLLM
    

    // TODO: Set PLLR

    
    // TODO: Enable PLLR output
    

    // TODO: Enable PLL
    
    
    // TODO: Wait until PLL is locked
    
}

void configureClock(){
    // Configure and turn on PLL
    configurePLL();

    // Select PLL as clock source
    RCC->CFGR |= (0b11 << 0);
    while(!((RCC->CFGR >> 2) & 0b11));
}