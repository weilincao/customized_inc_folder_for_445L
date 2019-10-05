//***********************  ESP8266.h  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Jonathan Valvano
 Sept 19, 2015
 
 */

#ifndef ESP8266_H
#define ESP8266_H

#define ESP8266_ENCRYPT_MODE_OPEN		 	      0
#define ESP8266_ENCRYPT_MODE_WEP		 	      1
#define ESP8266_ENCRYPT_MODE_WPA_PSK	 	    2
#define ESP8266_ENCRYPT_MODE_WPA2_PSK	 	    3
#define ESP8266_ENCRYPT_MODE_WPA_WPA2_PSK	  4
 
#define ESP8266_WIFI_MODE_CLIENT            1
#define ESP8266_WIFI_MODE_AP                2
#define ESP8266_WIFI_MODE_AP_AND_CLIENT     3

#define BUFFER_SIZE 2048

void DelayMs(uint32_t n);
//-------------------ESP8266_Init --------------
// Initialize the ESP module as client to access internet
// Inputs: none
// Outputs: none
void ESP8266_Init(void);

//---------ESP8266SendPacket-----
// Sends a packet to the ESP8266
// Inputs: string to send (null-terminated)
// Outputs: none
void ESP8266_SendPacket(char *outString);

//---------ESP8266StartSend-----
// Begin transmission to ESP8266 using TCP buffer
// Busy-wait until ESP8266 is ready to accept data
// Inputs: Size of data in bytes
// Outputs: none
void ESP8266_StartSend(int numBytes);

//---------ESP8266_SendChar-----
// Send a single byte to the ESP8266
// Inputs: 8-bit character or data value
// Outputs: none
void ESP8266_SendChar(char);

//---------ESP8266_FindResponse-----
// Look for response in RXBuffer
// Inputs: A string and length to search for
// Outputs: index of response or -1 if not found
int ESP8266_FindResponse(char *str, int length);

//---------ESP8266_GetAudio-----
// Return index to audio data in RXBuffer
// Assuming 800 samples are sent
// Busy-wait until response received
// Inputs: none
// Outputs: index into RXBuffer
int ESP8266_GetAudio(void);

//---------ESP8266_GetRXBuffer-----
// Return address of RXBuffer for audio output
// Inputs: none
// Outputs: memory address of RXBuffer
char *ESP8266_GetRXBuffer(void);

//---------ESP8266_CheckConnection-----
// Checks that the client has connected at 0
// Busy-wait until ESP8266 confirms connection
// Inputs: none
// Outputs: 1 if yes, 0 if no
int ESP8266_CheckConnection(void);

//-------------------ESP8266_InitClientServer --------------
// Initialize the ESP module as server or client
// Inputs: none
// Outputs: none
void ESP8266_InitClientServer(void);
void ESP8266_InitServer(void);
void ESP8266_InitClient(void);

//----------ESP8266_Reset------------
// Reset the ESP8266 module
// input:  none
// output: 1 if success, 0 if fail
int ESP8266_Reset(void);

//----------ESP8266_Restore------------
// Restore the ESP8266 module
// input:  none
// output: 1 if success, 0 if fail
int ESP8266_Restore(void);

//---------ESP8266_SetWifiMode----------
// Configure the ESP8266 to operate as both in Soft-AP mode
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_SetWifiMode(void);

//---------ESP8266_SetConnectionMux----------
// Enable the esp8266 connection mux, required for starting tcp server
// Input: none
// output: none
void ESP8266_EnableMux(void);

//---------ESP8266_ServerTimeout----------
// Sets the server timeout to five minutes
// Input: none
// output: none
void ESP8266_ServerTimeout(void);

//---------ESP8266_MakeHardTCPConnection-----
// Start TCP Connection with 5-minute keep-alive and confirm
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_MakeHardTCPConnection(void);

//---------ESP8266_MakeTCPConnection----------
// Establish TCP connection 
// Input: IP address or web page as a string
// output: 1 if success, 0 if fail 
int ESP8266_MakeTCPConnection(char *IPaddress);

//---------ESP8266_SendTCP----------
// Send a TCP packet to server
// assume this is a client
// Input: TCP payload to send
// output: 1 if success, 0 if fail 
int ESP8266_SendTCP(char* fetch);

//---------ESP8266_CloseTCPConnection----------
// Close TCP connection 
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_CloseTCPConnection(void);

//---------ESP8266_EnableServer----------
// Enables tcp server
// Input: none 
// output: none
void ESP8266_EnableServer(void);

//---------ESP8266_CIPClose----------
// Close all TCP connections
// Input: none 
// output: none
void ESP8266_CIPClose(void);

//---------ESP8266_JoinAccessPoint-----
// Connect to access point
// Inputs: Network name and password
// Outputs: 1 if success, 0 if fail
int ESP8266_JoinAccessPoint(const char* ssid, const char* password);

//---------ESP8266_QuitAccessPoint-----
// Disconnect from currently connected AP
// Inputs: none
// Outputs: none
void ESP8266_QuitAccessPoint(void);


//---------ESP8266_GetStatus----------
// Get Connection Status
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_GetStatus(void);

//---------ESP8266_GetVersionNumber----------
// Get Firmware Version Number (AT+GMR)
// Input: none
// output: 1 if success, 0 if fail 
int ESP8266_GetVersionNumber(void);
#endif
