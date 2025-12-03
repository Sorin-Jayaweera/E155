// DFPLAYER_MINI.c
// DFPlayer Mini MP3 module control implementation
// Uses hardware USART1 (PA9/PA10) - ORIGINAL WORKING VERSION

#include "../lib/STM32L432KC.h"
#include "DFPLAYER_MINI.h"

// DFPlayer protocol constants (from original DFPLAYER_MINI.c)
#define Start_Byte  0x7E
#define End_Byte    0xEF
#define Version     0xFF
#define Cmd_Len     0x06
#define Feedback    0x00  // No feedback
#define Source      0x02  // TF CARD

// Button pins (ORIGINAL configuration)
#define Previous_Key  8   // PA8
#define Pause_Key     6   // PA6
#define Next_Key      7   // PB7

// State variables (from original DFPLAYER_MINI.c)
static int isPaused = 0;
static int isPlaying = 1;

/**
 * @brief Millisecond delay using busy-wait loop
 * @note TIM15 is used for FFT synthesis, so we use busy-wait instead
 */
static void delay_millis(uint32_t ms) {
    // At 80 MHz: ~80000 cycles per millisecond
    volatile uint32_t cycles = ms * 16000;  // Approximate
    while (cycles--);
}

/**
 * @brief Send command to DFPlayer (from original DFPLAYER_MINI.c)
 * @param USART USART peripheral (USART1)
 * @param cmd Command byte
 * @param Parameter1 Parameter 1
 * @param Parameter2 Parameter 2
 */
static void Send_cmd(USART_TypeDef * USART, uint8_t cmd, uint8_t Parameter1, uint8_t Parameter2) {
    uint16_t Checksum = Version + Cmd_Len + cmd + Feedback + Parameter1 + Parameter2;
    Checksum = 0 - Checksum;

    uint8_t CmdSequence[10] = {
        Start_Byte,
        Version,
        Cmd_Len,
        cmd,
        Feedback,
        Parameter1,
        Parameter2,
        (Checksum >> 8) & 0x00FF,
        (Checksum & 0x00FF),
        End_Byte
    };

    // Send each byte using hardware USART (from E155 library)
    for (int i = 0; i < 10; i++) {
        sendChar(USART, CmdSequence[i]);
    }
}

/**
 * @brief Initialize DFPlayer Mini (from original DFPLAYER_MINI.c)
 * @param USART USART peripheral (USART1)
 * @param volume Initial volume (0-30)
 */
void DF_Init(USART_TypeDef * USART, uint8_t volume) {
    // Wait for DFPlayer to boot (from original)
    delay_millis(500);

    // Initialize DFPlayer (from original)
    Send_cmd(USART, 0x3F, 0x00, Source);  // Set source to TF card
    delay_millis(200);
    Send_cmd(USART, 0x06, 0x00, volume);  // Set volume
    delay_millis(500);
}

/**
 * @brief Play from start (from original DFPLAYER_MINI.c)
 */
void DF_PlayFromStart(USART_TypeDef * USART) {
    Send_cmd(USART, 0x03, 0x00, 0x01);  // Play first track
    delay_millis(200);
    isPlaying = 1;
    isPaused = 0;
}

/**
 * @brief Next track (from original DFPLAYER_MINI.c)
 */
void DF_Next(USART_TypeDef * USART) {
    Send_cmd(USART, 0x01, 0x00, 0x00);  // Next track
    delay_millis(200);
}

/**
 * @brief Previous track (from original DFPLAYER_MINI.c)
 */
void DF_Previous(USART_TypeDef * USART) {
    Send_cmd(USART, 0x02, 0x00, 0x00);  // Previous track
    delay_millis(200);
}

/**
 * @brief Pause playback (from original DFPLAYER_MINI.c)
 */
void DF_Pause(USART_TypeDef * USART) {
    Send_cmd(USART, 0x0E, 0x00, 0x00);  // Pause
    delay_millis(200);
}

/**
 * @brief Resume playback (from original DFPLAYER_MINI.c)
 */
void DF_Playback(USART_TypeDef * USART) {
    Send_cmd(USART, 0x0D, 0x00, 0x00);  // Resume playback
    delay_millis(200);
}

/**
 * @brief Set volume (from original DFPLAYER_MINI.c)
 */
void DF_SetVolume(USART_TypeDef * USART, uint8_t volume) {
    Send_cmd(USART, 0x06, 0x00, volume);  // Set volume (0-30)
    delay_millis(200);
}

/**
 * @brief Play specific track (from original DFPLAYER_MINI.c)
 */
void DF_PlayTrack(USART_TypeDef * USART, uint8_t track) {
    Send_cmd(USART, 0x03, 0x00, track);  // Play specific track
    delay_millis(200);
}

/**
 * @brief Check buttons and send DFPlayer commands (from original DFPLAYER_MINI.c)
 * @param USART USART peripheral (USART1)
 */
void Check_Key(USART_TypeDef * USART) {
    // Check pause/play button (PA6) - from original
    if (digitalRead(Pause_Key)) {
        // Wait for button release (debounce) - from original
        while (digitalRead(Pause_Key));
        delay_millis(50);  // Additional debounce

        if (isPlaying) {
            isPaused = 1;
            isPlaying = 0;
            DF_Pause(USART);
        } else if (isPaused) {
            isPlaying = 1;
            isPaused = 0;
            DF_Playback(USART);
        }
    }

    // Check previous button (PA8) - from original
    if (digitalRead(Previous_Key)) {
        while (digitalRead(Previous_Key));
        delay_millis(50);
        DF_Previous(USART);
    }

    // Check next button (PB7) - from original
    uint8_t next_reading = (GPIOB->IDR >> Next_Key) & 1;
    if (next_reading) {
        while ((GPIOB->IDR >> Next_Key) & 1);
        delay_millis(50);
        DF_Next(USART);
    }
}
