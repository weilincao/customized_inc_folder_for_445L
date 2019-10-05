// WifiSettings.h
// Dung Nguyen
// Wally Guzman

// Launchpad or TM4C123
#define LAUNCHPAD 		1

// Client or Server ESP8266 Initialization
// 0 means client, 1 means server
#define SERVER 				0
 
// Baudrate for UART connection to ESP8266
#define BAUDRATE 			115200

// ADC Sample Rate in Hz
#define SAMPLERATE		8000

// Return values
#define NORESPONSE		(-1)
#define BADVALUE			(-1)
#define SUCCESS				1
#define FAILURE				0

enum Menu_Status {RX=0, TX, CONNECTED};
	
// Access Point Parameters
//#define SSID  		 		"AI-THINKER_E465C7"

#define SSID_NAME  "YourAP"
#define PASSKEY    "YourPassword"
