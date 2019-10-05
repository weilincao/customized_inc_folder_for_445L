// esp8266v2.c
// Brief description of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//

// http://www.xess.com/blog/esp8266-reflash/

// NOTE: ESP8266 resources below:
// General info and AT commands: http://nurdspace.nl/ESP8266
// General info and AT commands: http://www.electrodragon.com/w/Wi07c
// Community forum: http://www.esp8266.com/
// Offical forum: http://bbs.espressif.com/
// example http://zeflo.com/2014/esp8266-weather-display/

/* Modified by Jonathan Valvano
 September 21, 2016
 Hardware connections
 Vcc is a separate regulated 3.3V supply with at least 215mA
 /------------------------------\
 |              chip      1   8 |
 | Ant                    2   7 |
 | enna       processor   3   6 |
 |                        4   5 |
 \------------------------------/ 
ESP8266    TM4C123 
  1 URxD    PB1   UART out of TM4C123, 115200 baud
  2 GPIO0         +3.3V for normal operation (ground to flash)
  3 GPIO2         +3.3V
  4 GND     Gnd   GND (70mA)
  5 UTxD    PB0   UART out of ESP8266, 115200 baud
  6 Ch_PD         chip select, 10k resistor to 3.3V
  7 Reset   PB5   TM4C123 can issue output low to cause hardware reset
  8 Vcc           regulated 3.3V supply with at least 70mA
 */
// modified by Dung Nguyen, Wally Guzman
/*
===========================================================
==========          CONSTANTS                    ==========
===========================================================
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/esp8266v2.h"
#include "../inc/UART1.h"
#include "../inc/WifiSettings.h"

// Prototypes for helper functions
void DelayMs(uint32_t delay);
void ESP8266SendCommand(char* inputString);
void ESP8266FIFOtoBuffer(void);
void ESP8266_WaitForResponse(char *str, int length);

unsigned int ESP8266_GetUInt(void);

/***************GLOBALS***************/

uint32_t RXReadIndex = 0;
uint32_t RXWriteIndex = 0;
char RXBuffer[BUFFER_SIZE];
char TXBuffer[BUFFER_SIZE];
bool ResponseReceived = false;

void DelayMs(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 6665;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
}

/**************INITIALIZATION****************/
//-------------------ESP8266_Init --------------
// Initialize the ESP module as client to access internet
// Inputs: none
// Outputs: none
void ESP8266_Init(void){
 	UART1_Init(BAUDRATE); // initialize UART
	ESP8266_Restore();
	ESP8266_Reset();
	ESP8266_SetWifiMode();
	ESP8266_JoinAccessPoint(SSID_NAME, PASSKEY); // Pass in strings with #define
}
//-------------------ESP8266_InitClientServer --------------
// Initialize the ESP module as server or client to connect two ESP8266s
// Inputs: none
// Outputs: none
void ESP8266_InitClientServer(void){
 	UART1_Init(BAUDRATE); // initialize UART
	ESP8266_Restore();
	ESP8266_Reset();
	ESP8266_SetWifiMode();

	// Choose Client/Server
	(SERVER) ? ESP8266_InitServer() : ESP8266_InitClient();
}


void ESP8266_InitServer(void){
	ESP8266_EnableMux();
	ESP8266_EnableServer();
	ESP8266_ServerTimeout();
}

void ESP8266_InitClient(void){
	ESP8266_JoinAccessPoint(""SSID_NAME, ""PASSKEY); // Pass in strings with #define
	ESP8266_MakeHardTCPConnection();
}

/*************UART1_Handler****************/
// called on one receiver data input followed by timeout
// or on going from 1 to 2 data input characters
void UART1_Handler(void){
  if(UART1_RIS_R & UART_RIS_RXRIS){   // rx fifo >= 1/8 full
    UART1_ICR_R = UART_ICR_RXIC;      // acknowledge interrupt
    ESP8266FIFOtoBuffer();
		ResponseReceived = true;
  }
  if(UART1_RIS_R & UART_RIS_RTRIS){   // receiver timed out
    UART1_ICR_R = UART_ICR_RTIC;      // acknowledge receiver time
    ESP8266FIFOtoBuffer();
  }
}

//----------ESP8266FIFOtoBuffer----------
// Transfer data from ESP8266 over UART to software buffer
void ESP8266FIFOtoBuffer(void){
	while((UART1_FR_R&UART_FR_RXFE) == 0){
		RXBuffer[RXWriteIndex] = UART1_DR_R;
		RXWriteIndex++;
		if(RXWriteIndex >= BUFFER_SIZE) RXWriteIndex = 0;
	}
}

/***********SEND COMMANDS*****************/
//---------ESP8266SendCommand-----
// Sends a command to the ESP8266 module
// however, hardware has 16 character hardware FIFO
// Inputs: string to send (null-terminated)
// Outputs: none
void ESP8266SendCommand(char* command){
  UART1_OutString(command);
}

//---------ESP8266StartSend-----
// Begin transmission to ESP8266 using TCP buffer
// Busy-wait until ESP8266 is ready to accept data
// Inputs: Size of data in bytes
// Outputs: none
void ESP8266_StartSend(int numBytes) {
	char command[50];
	
	if (SERVER)
		sprintf(command, "AT+CIPSENDBUF=%d,%d\r\n", 0, numBytes);
	else
		sprintf(command, "AT+CIPSENDBUF=%d\r\n", numBytes);
	
	UART1_OutString(command);
	ESP8266_WaitForResponse(">", 1);
}


//---------ESP8266SendPacket-----
// Sends a packet to the ESP8266
// Inputs: string to send (null-terminated)
// Outputs: none
void ESP8266_SendPacket(char *outString) {
	char command[50];
	
	if (SERVER)
		sprintf(command, "AT+CIPSENDBUF=%d,%d\r\n", 0, strlen(outString));
	else
		sprintf(command, "AT+CIPSENDBUF=%d\r\n", strlen(outString));
	
	UART1_OutString(command);
	ESP8266_WaitForResponse(">", 1);
	UART1_OutString(outString);	
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_SendChar-----
// Send a single byte to the ESP8266
// Inputs: 8-bit character or data value
// Outputs: none
void ESP8266_SendChar(char letter){
	(SERVER) ? UART1_OutString("AT+CIPSEND=0,1\r\n") : UART1_OutString("AT+CIPSEND=1\r\n");
	UART1_OutChar(letter);
}

//---------ESP8266_GetString-----
// Copy string received from ESP8266 into buffer
// Inputs: buffer and length
// Outputs: none
void ESP8266_GetString(char* out, unsigned int length){
	for(int k = 0; k < length; k++)
		out[k] = RXBuffer[(RXReadIndex + k) % BUFFER_SIZE];
	out[length] = 0;
}

//---------ESP8266_CheckConnection-----
// Checks that the client has connected at 0
// Busy-wait until ESP8266 confirms connection
// Inputs: none
// Outputs: 1 if yes, 0 if no
int ESP8266_CheckConnection(void) {
	int index = ESP8266_FindResponse("0,CONNECT", 9);
	if(index != NORESPONSE) {
		RXReadIndex = index;
		return SUCCESS;
	}
	return FAILURE;
}

//---------ESP8266_GetAudio-----
// Return index to audio data in RXBuffer
// Assuming 800 samples are sent
// Busy-wait until response received
// Inputs: none
// Outputs: index into RXBuffer
int ESP8266_GetAudio(void) {
	int index = (SERVER) ? ESP8266_FindResponse("+IPD,0,800:", 11) : ESP8266_FindResponse("+IPD,800:", 9);
	if(index != NORESPONSE) {
		RXReadIndex = index + 800;
		return index;
	} 
	return NORESPONSE;
}

//---------ESP8266_GetRXBuffer-----
// Return address of RXBuffer for audio output
// Inputs: none
// Outputs: memory address of RXBuffer
char *ESP8266_GetRXBuffer(void) {
	return RXBuffer;
}

/***********PROCESSING*******************/
//---------CheckString-----
// Checks for matching string in buffer
// Inputs: index in RXBuffer, a string to match, and a length
// Outputs: number of characters matched
int CheckString(int index, char *str, int length) {
	int charsMatched = 0;
	while((str[charsMatched] == RXBuffer[index % BUFFER_SIZE]) && 
					charsMatched < length) {
		index ++;
		charsMatched++;
	}
	return charsMatched;
}

//---------ESP8266_FindResponse-----
// Look for response in RXBuffer
// Inputs: A string and length to search for
// Outputs: index of response or -1 if not found
int ESP8266_FindResponse(char *str, int length){
	int temp = RXReadIndex;
	int WIndex = RXWriteIndex;
	if(RXReadIndex > RXWriteIndex)
		WIndex += BUFFER_SIZE;
	while((temp < WIndex) && (CheckString(temp, str, length) < length)){
		temp++;
	}
	if(temp < WIndex)
		return (temp + length) % BUFFER_SIZE;
	else
		return NORESPONSE;
}

//---------ESP8266_WaitForResponse-----
// Busy-wait until response found
// Inputs: A string and length to search for
// Outputs: none
void ESP8266_WaitForResponse(char *str, int length) {
	int index = ESP8266_FindResponse(str, length);
	while(index == NORESPONSE) {
		index = ESP8266_FindResponse(str, length);
	}
	RXReadIndex = index;
}

//---------ESP8266_Restore-----
// Restore the ESP8266 module to default values
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_Restore(void) {
	UART1_OutString("+++\r\n");
	ESP8266_WaitForResponse("ERROR", 5);
	UART1_OutString("AT+RESTORE\r\n");
	ESP8266_WaitForResponse("ready", 5);
	return SUCCESS;
}

//---------ESP8266_Restore-----
// Reset the ESP8266 module
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_Reset(void) {
	/*GPIO_PORTB_DATA_R &= ~0x20; 	// reset low
	DelayMs(10);
	GPIO_PORTB_DATA_R |= 0x20; 		// reset high
  DelayMs(10);*/
	UART1_OutString("AT+RST\r\n");
	ESP8266_WaitForResponse("ready", 5);
	return SUCCESS;
}

//---------ESP8266_SetWifiMode-----
// Set Wifi mode of ESP8266 to soft-AP mode, both client and server
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_SetWifiMode(void) {
	UART1_OutString("AT+CWMODE=3\r\n");
	ESP8266_WaitForResponse("OK", 2);
	return SUCCESS;
}

//---------ESP8266_JoinAccessPoint-----
// Connect to access point
// Inputs: Network name and password
// Outputs: 1 if success, 0 if fail
int ESP8266_JoinAccessPoint(const char* ssid, const char* password) {
	char ap[50]= {0};
	sprintf(ap, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
	UART1_OutString(ap);
	ESP8266_WaitForResponse("WIFI GOT IP", 11);
	ESP8266_WaitForResponse("OK", 2);
	UART1_OutString("AT+CIFSR\r\n");
	ESP8266_WaitForResponse("192.168.4.2", 11);
	ESP8266_WaitForResponse("OK", 2);
	return SUCCESS;
}

//---------ESP8266_MakeHardTCPConnection-----
// Start TCP Connection with 5-minute keep-alive and confirm
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_MakeHardTCPConnection(void) {
	char tcp[50]= {0};
	sprintf(tcp, "AT+CIPSTART=\"TCP\",\"192.168.4.1\",333,600\r\n"); // 600 = 5 minutes keep-alive
	UART1_OutString(tcp);
	ESP8266_WaitForResponse("OK", 2);
	UART1_OutString("AT+CIPSTATUS\r\n");
	ESP8266_WaitForResponse("STATUS:3", 8);
	return SUCCESS;
}
//---------ESP8266_MakeTCPConnection----------
// Establish TCP connection 
// Input: IP address or web page as a string
// output: 1 if success, 0 if fail 
int ESP8266_MakeTCPConnection(char *IPaddress){ 
  sprintf((char*)TXBuffer, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", IPaddress);
  ESP8266SendCommand(TXBuffer);   // open and connect to a socket
	ESP8266_WaitForResponse("OK", 2);
  return 1; // success
}
//---------ESP8266_SendTCP----------
// Send a TCP packet to server
// assume this is a client
// Input: TCP payload to send
// output: 1 if success, 0 if fail 
int ESP8266_SendTCP(char* fetch){
  volatile uint32_t time,n;
  sprintf((char*)TXBuffer, "AT+CIPSEND=%d\r\n", strlen(fetch));
  ESP8266SendCommand(TXBuffer);  
  DelayMs(50);
  ESP8266SendCommand(fetch);
  ESP8266_WaitForResponse("+ipd,", 5);
  return 1; // success
}

//---------ESP8266_CloseTCPConnection----------
// Close TCP connection 
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_CloseTCPConnection(void){
  ESP8266SendCommand("AT+CIPCLOSE\r\n");   
	ESP8266_WaitForResponse("OK", 2);
  return 1; // success
}

//---------ESP8266_EnableMux-----
// Set multiple connections on ESP8266
// Inputs: none
// Outputs: none
void ESP8266_EnableMux(void) {
	UART1_OutString("AT+CIPMUX=1\r\n"); // Set to 0 to check single socket
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_EnableServer-----
// Enable ESP8266 as server
// Inputs: none
// Outputs: none
void ESP8266_EnableServer(void) {
	UART1_OutString("AT+CIPSERVER=1,333\r\n");
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_QuitAccessPoint-----
// Disconnect from currently connected AP
// Inputs: none
// Outputs: none
void ESP8266_QuitAccessPoint(void) {
	UART1_OutString("AT+CWQAP\r\n");
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_ServerTimeout-----
// Set server timeout to five-minutes
// Inputs: none
// Outputs: none
void ESP8266_ServerTimeout(void) {
	UART1_OutString("AT+CIPSTO=600\r\n"); // 600 >= 5 minutes keep-alive
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_CIPClose-----
// Close all TCP connections
// Inputs: none
// Outputs: none
void ESP8266_CIPClose(void) {
	UART1_OutString("AT+CIPCLOSE=5\r\n"); // ID=5 closes all connections
	ESP8266_WaitForResponse("OK", 2);
}

//---------ESP8266_GetStatus----------
// Get Connection Status
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_GetStatus(void){
	UART1_OutString("AT+CIPSTATUS\r\n");
	ESP8266_WaitForResponse("OK", 2);
	return SUCCESS;
}

//---------ESP8266_GetVersionNumber----------
// Get Firmware Version Number (AT+GMR)
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_GetVersionNumber(void){
	UART1_OutString("AT+GMR\r\n");
	ESP8266_WaitForResponse("OK", 2);
	return SUCCESS;
}
