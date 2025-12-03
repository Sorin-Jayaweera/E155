// DFPLAYER_MINI.c
// DFPlayer Mini MP3 module control implementation
// Adapted from working DFPLAYER_MINI.c with software UART for E155 PCB

#include "DFPLAYER_MINI.h"

/*******************************************************************************
 * SOFTWARE UART FOR DFPLAYER MINI
 * Bit-banged UART TX on PA10 @ 9600 baud (no RX needed)
 * Replaces hardware USART for pin compatibility with FFT code
 ******************************************************************************/

#define SOFT_UART_TX_PIN 10  // PA10
#define BAUD_RATE 9600
#define BIT_DELAY_US (1000000 / BAUD_RATE)  // ~104 us per bit @ 9600 baud

/**
 * @brief Microsecond delay for software UART bit timing
 */
static void delay_us(uint32_t us) {
    // At 80 MHz: 80 cycles per microsecond
    // Account for loop overhead (~4 cycles per iteration)
    volatile uint32_t cycles = us * 20;  // Approximate: 80/4 = 20
    while (cycles--);
}

/**
 * @brief Send one byte via software UART (8N1 format)
 * @note Replaces sendChar(USART, byte) from E155 library
 */
static void sendChar(uint8_t byte) {
    // START bit (low)
    digitalWrite(SOFT_UART_TX_PIN, GPIO_LOW);
    delay_us(BIT_DELAY_US);

    // 8 data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        if (byte & (1 << i)) {
            digitalWrite(SOFT_UART_TX_PIN, GPIO_HIGH);
        } else {
            digitalWrite(SOFT_UART_TX_PIN, GPIO_LOW);
        }
        delay_us(BIT_DELAY_US);
    }

    // STOP bit (high)
    digitalWrite(SOFT_UART_TX_PIN, GPIO_HIGH);
    delay_us(BIT_DELAY_US);
}

/**
 * @brief Millisecond delay using busy-wait loop
 * @note Replaces delay_millis(TIM15, ms) from E155 library
 *       TIM15 is reserved for FFT synthesis, so we can't use it for delays
 */
static void delay_millis(uint32_t ms) {
    // At 80 MHz: ~80000 cycles per millisecond
    volatile uint32_t cycles = ms * 16000;  // Approximate
    while (cycles--);
}

/*******************************************************************************
 * DFPLAYER MINI PROTOCOL
 ******************************************************************************/

// DFPlayer protocol constants (from original DFPLAYER_MINI.c)
#define Start_Byte  0x7E
#define End_Byte    0xEF
#define Version     0xFF
#define Cmd_Len     0x06
#define Feedback    0x00  // No feedback
#define Source      0x02  // TF CARD

// Button pins (remapped for E155 PCB)
#define Previous_Key  8   // PA8 (same as original)
#define Pause_Key     7   // PB7 (swapped with Next)
#define Next_Key      0   // PB0 (moved from PA6 - ADC conflict!)

// State variables (from original DFPLAYER_MINI.c)
static int isPaused = 0;
static int isPlaying = 1;

/**
 * @brief Send command to DFPlayer (from original DFPLAYER_MINI.c)
 */
static void Send_cmd(uint8_t cmd, uint8_t Parameter1, uint8_t Parameter2) {
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

    // Send each byte using software UART
    for (int i = 0; i < 10; i++) {
        sendChar(CmdSequence[i]);
    }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void DF_Init(uint8_t volume) {
    // Wait for DFPlayer to boot (from original)
    delay_millis(500);

    // Initialize DFPlayer (from original)
    Send_cmd(0x3F, 0x00, Source);  // Set source to TF card
    delay_millis(200);
    Send_cmd(0x06, 0x00, volume);  // Set volume
    delay_millis(500);
}

void DF_PlayFromStart(void) {
    Send_cmd(0x03, 0x00, 0x01);  // Play first track
    delay_millis(200);
    isPlaying = 1;
    isPaused = 0;
}

void DF_Next(void) {
    Send_cmd(0x01, 0x00, 0x00);  // Next track
    delay_millis(200);
}

void DF_Previous(void) {
    Send_cmd(0x02, 0x00, 0x00);  // Previous track
    delay_millis(200);
}

void DF_Pause(void) {
    Send_cmd(0x0E, 0x00, 0x00);  // Pause
    delay_millis(200);
}

void DF_Playback(void) {
    Send_cmd(0x0D, 0x00, 0x00);  // Resume playback
    delay_millis(200);
}

void DF_SetVolume(uint8_t volume) {
    Send_cmd(0x06, 0x00, volume);  // Set volume (0-30)
    delay_millis(200);
}

void DF_PlayTrack(uint8_t track) {
    Send_cmd(0x03, 0x00, track);  // Play specific track
    delay_millis(200);
}

void Check_Key(void) {
    // Check pause/play button (PB0 instead of PA6)
    uint8_t pause_reading = (GPIOB->IDR >> Pause_Key) & 1;
    if (pause_reading) {
        // Wait for button release (debounce) - from original
        while ((GPIOB->IDR >> Pause_Key) & 1);
        delay_millis(50);  // Additional debounce

        if (isPlaying) {
            isPaused = 1;
            isPlaying = 0;
            DF_Pause();
        } else if (isPaused) {
            isPlaying = 1;
            isPaused = 0;
            DF_Playback();
        }
    }

    // Check previous button (PA8) - from original
    if (digitalRead(Previous_Key)) {
        while (digitalRead(Previous_Key));
        delay_millis(50);
        DF_Previous();
    }

    // Check next button (PB7) - from original
    uint8_t next_reading = (GPIOB->IDR >> Next_Key) & 1;
    if (next_reading) {
        while ((GPIOB->IDR >> Next_Key) & 1);
        delay_millis(50);
        DF_Next();
    }
}
