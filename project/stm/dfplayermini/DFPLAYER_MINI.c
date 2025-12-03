// DFPLAYER_MINI.c
// DFPlayer Mini MP3 module control implementation
// Uses E155 USART library

#include "DFPLAYER_MINI.h"

// DFPlayer protocol constants
#define Start_Byte  0x7E
#define End_Byte    0xEF
#define Version     0xFF
#define Cmd_Len     0x06
#define Feedback    0x00  // No feedback

#define Source      0x02  // TF CARD

// Button pins
#define Previous_Key  PA8
#define Pause_Key     PA6
#define Next_Key      PB7


// State variables
static int isPaused = 0;
static int isPlaying = 1;

// Send command to DFPlayer
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
    
    // Send each byte using E155 USART library
    for (int i = 0; i < 10; i++) {
        sendChar(USART, CmdSequence[i]);
    }
}

void DF_Init(USART_TypeDef * USART, uint8_t volume) {
    // Initialize button pins
    pinMode(Previous_Key, GPIO_INPUT);
    pinMode(Pause_Key, GPIO_INPUT);
    pinMode(Next_Key, GPIO_INPUT);

    // Wait for DFPlayer to boot
    delay_millis(TIM2, 500);

    // Initialize DFPlayer
    Send_cmd(USART, 0x3F, 0x00, Source);  // Set source to TF card
    delay_millis(TIM2, 200);
    Send_cmd(USART, 0x06, 0x00, volume);  // Set volume
    delay_millis(TIM2, 500);
}

void DF_PlayFromStart(USART_TypeDef * USART) {
    Send_cmd(USART, 0x03, 0x00, 0x01);  // Play first track
    delay_millis(TIM2, 200);
    isPlaying = 1;
    isPaused = 0;
}

void DF_Next(USART_TypeDef * USART) {
    Send_cmd(USART, 0x01, 0x00, 0x00);  // Next track
    delay_millis(TIM2, 200);
}

void DF_Previous(USART_TypeDef * USART) {
    Send_cmd(USART, 0x02, 0x00, 0x00);  // Previous track
    delay_millis(TIM2, 200);
}

void DF_Pause(USART_TypeDef * USART) {
    Send_cmd(USART, 0x0E, 0x00, 0x00);  // Pause
    delay_millis(TIM2, 200);
}

void DF_Playback(USART_TypeDef * USART) {
    Send_cmd(USART, 0x0D, 0x00, 0x00);  // Resume playback
    delay_millis(TIM2, 200);
}

void DF_SetVolume(USART_TypeDef * USART, uint8_t volume) {
    Send_cmd(USART, 0x06, 0x00, volume);  // Set volume (0-30)
    delay_millis(TIM2, 200);
}

void DF_PlayTrack(USART_TypeDef * USART, uint8_t track) {
    Send_cmd(USART, 0x03, 0x00, track);  // Play specific track
    delay_millis(TIM2, 200);
}

void Check_Key(USART_TypeDef * USART) {
    // Check pause/play button
    if (digitalRead(Pause_Key)) {
        // Wait for button release (debounce)
        while (digitalRead(Pause_Key));
        delay_millis(TIM2, 50);  // Additional debounce

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

    // Check previous button
    if (digitalRead(Previous_Key)) {
        while (digitalRead(Previous_Key));
        delay_millis(TIM2, 50);
        DF_Previous(USART);
    }

    // Check next button
    if (digitalRead(Next_Key)) {
        while (digitalRead(Next_Key));
        delay_millis(TIM2, 50);
        DF_Next(USART);
    }
}