/*
File: Lab_6_JHB.c
Author: Sorin Jaywaeera
Email: sojayaweera@hmc.edu
Date: 10/19/25
*/
//MIKRO ADDR 
// 27

#include "STM32L432KC.h"
#include "STM32L432KC_SPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define LED_PIN PA5
#define BUFF_LEN 32

//#define SPI_CE PA11
//#define SPI_SCK PB3
//#define SPI_MOSI PB5
//#define SPI_MISO PB4

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";

char* resStr = "<p>Resolution control:</p><form action=\"res8\"><input type=\"submit\" value=\"8 bit\"></form>\
        <form action=\"res9\"><input type=\"submit\" value=\"9 bit\"></form>\
        <form action=\"res10\"><input type=\"submit\" value=\"10 bit\"></form>\
        <form action=\"res11\"><input type=\"submit\" value=\"11 bit\"></form>\
        <form action=\"res12\"><input type=\"submit\" value=\"12 bit\"></form>";

char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}


// temperature sensor resolution
int tempres;
int templastres; // previous to see if it changes
//
int updateTempResolution(char request[]){
  int resolution = 8;
  if(inString(request,"res8") == 1){resolution = 8;}
  if(inString(request,"res9") == 1){resolution = 9;}
  if(inString(request,"res10") == 1){resolution = 10;}
  if(inString(request,"res11") == 1){resolution = 11;}
  if(inString(request,"res12") == 1){resolution = 12;}

  return resolution;
  
}
/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

int main(void) {
  configureFlash();
  configureClock();

  // br: needs to yield 5 MHz. 80 MHz SYSCLK/5MHZ = 16, option 011
  // cpol: don't care. Choose 0 when idle
  // cpha: first clock edge
  // br, cpol, cpha
  initSPI( 0b011, 0,0); 

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);
  
  initTIM(TIM15);

  pinMode(LED_PIN, GPIO_OUTPUT);
  digitalWrite(LED_PIN, 0);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */
    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }
  
    // Update string with current LED state
    int led_status = updateLEDStatus(request);
    
    //tempres = updateTempResolution(request);
    
    // IF THE USER WANTS TO CHANGE THE RESOLUTION
    //if(templastres != templastres){
    //  templastres = tempres;
      
    //  // enable communication
    //  digitalWrite(SPI_CE, 1); // enable high
    //  int resReg = 0;

    //  // most to least significant bits.
    //  // 111 1Shot(0) ### SD(0)
    //  switch(tempres){
    //    spiSendReceive(0x80);
    //    case 8:
    //      spiSendReceive(0b11100000); // 000 for 8 bit
    //    case 9:
    //      spiSendReceive(0b11100010); // 001 9 bit
    //    case 10:
    //      spiSendReceive(0b11100100); // 010
    //    case 11: 
    //      spiSendReceive(0b11100110); // 011
    //    case 12:
    //      spiSendReceive(0b11101000); // 1xx
    //  }
      
    //  digitalWrite(SPI_CE, 0); // 
    //}

    // read temperature
    // 01 LSB
    // 02 MSB
    //digitalWrite(SPI_CE, 1); // 
    //char LSB = spiSendReceive(0x1);// addr 1
    //char MSB = spiSendReceive(0x2);// addr 2
    //double temperature;
    //// above zero powers of two
    //for (int i = 7; i >=0; i--){
    //  int bit = (1 << i) & MSB;
    //  temperature = temperature + bit *pow(2,(7-i));    
    //}
    //// below zero powers of two
    //for (int i = 7; i >=4; i--){
    //  int bit = (1 << i) & LSB;
    //  temperature = temperature + bit *pow(2,(6-i));    
    //}

    //digitalWrite(SPI_CE, 0);



    //char temperaturebuffer[50];
    //sprintf(temperaturebuffer,"Temp (c): %.3f",temperature);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED

    sendString(USART, "<h2>LED Status</h2>");
    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");
    

    //sendString(USART, "<p>");
    //sendString(USART, temperaturebuffer);
    //sendString(USART,"</p>");
  
    sendString(USART, webpageEnd);
  }
}


