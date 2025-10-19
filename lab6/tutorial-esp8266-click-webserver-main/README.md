# ESP8266 Webserver Tutorial

This tutorial demonstrates how to interact with a ESP8266 webserver ([code](./ESP8266-IoT-webserver/)).
In short, the MCU does the following:

1. Connects over a 125000 baud UART connection to the ESP8266
2. Waits for a request to be sent over from the ESP8266
3. Parses the request with is in the form `REQ://<tag>` where `<tag>` must be a string less than 10 bytes long (this can be modified by increasing the size of the array on the MCU).
4. Performs computations in response to the parsed command.
5. Constructs and sends back a full, valid HTML webpage to the ESP8266 which subsequently displays it.