/*
 * main.c
 * DFPlayer Mini MP3 Player Controller
 * For STM32L432KC using Segger Embedded Studio
 * Uses E155 library structure
 * 
 * Hardware Connections:
 * - UART2 (DFPlayer): PA9 (TX) -> DFPlayer RX, PA10 (RX) -> DFPlayer TX
 * - Buttons: PA8 (Previous), PA6 (Pause/Play), PB7 (Next)
 * - DFPlayer: VCC (3.3V), GND, SPK+ and SPK- to speaker
 * 
 * Usage:
 * 1. Insert microSD card with MP3 files in root directory (001.mp3, 002.mp3, etc.)
 * 2. Connect DFPlayer Mini to STM32L432KC as shown above
 * 3. Connect speaker to SPK+ and SPK-
 * 4. Power on and press buttons to control playback
 */
//#include "STM32L432KC.h"
// #include "main.h"
#include "DFPLAYER_MINI.h"

int main(void) {
    // Configure flash and clock (sets system to 80 MHz)
    configureFlash();
    configureClock();

    // Enable GPIO ports
    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);

    // Enable TIM15 for delays
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    initTIM(TIM15);

    // Initialize USART2 at 9600 baud for DFPlayer communication
    // DFPlayer Mini requires 9600 baud rate
    USART_TypeDef * USART = initUSART(USART1_ID, 9600);

    // Initialize DFPlayer with volume 20 (range: 0-30)
    DF_Init(USART, 25);

    // Small delay to ensure initialization
    delay_millis(TIM15, 100);

    // Start playing from first track
    DF_PlayFromStart(USART);

    // Main loop - check for button presses
    while (1) {
        Check_Key(USART);
        
        // Small delay to prevent excessive polling
        delay_millis(TIM15, 10);
    }

    return 0;
}