// all about Network

#include <WiFi.h>
//#include <AsyncUDP.h>

#include <WiFiAP.h>
#include <WiFiClient.h>

//Declarations
void WiFi_Start_STA(void);


// Wifi variables & definitions
#define MAX_PACKAGE_SIZE 2048
char HTML_String[8000];
char HTTP_Header[150];

byte my_WiFi_Mode = 0;  // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
//---------------------------------------------------------------------
// Allgemeine Variablen

int Aufruf_Zaehler = 0;

#define ACTION_SET_SSID        1  
#define ACTION_SET_OUTPUT_TYPE 2  // also adress at EEPROM

int action;

char tmp_string[20];
//---------------------------------------------------------------------
