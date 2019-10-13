//version for Fendt 209V and Pellenc 3050 and Android RC APP
//running on LOLIN D32 ESP32
//2019-08-24
//by Matthias Hammer/MTZ8302. Webpage based on WEDER/coffeetrac

//settings page: 192.168.1.50
//to configure WIFI settings for existing network

//this ESP32 program handles UDP information, send by an Android App to operate the relays connected to the ESP
//it receives on port 3050 IP adress x.x.x.50 or x.x.x.255 by your network e.g. 192.168.1.50
//it sends the switch PGN to AOG x.x.x.255 to port 9999 from port 5555 to turn AOG (AgOpenGPS) section on/off


//##########################################################################################################
//### Setup Zone ###########################################################################################
// Just default values
//##########################################################################################################

#define timeoutRouter  40         // Time (seconds) to wait for WIFI access, after that own Access Point starts      
                              
struct Storage{    
	char ssid[24] = "Fendt_209V";          // WiFi network Client name
	char password[24] = "";      // WiFi network password

};  Storage PellencSettings;

//debug
bool Reset_EEPROM_settings = false;
bool debugmode = false;
bool RelayTest = false;
bool EE_done = 0;

//Accesspoint name and password:
const char* ssid_ap     = "Pellenc_3050_AP";
const char* password_ap = "";

//static IP for Accesspoint
IPAddress myAPip(192, 168, 1, 50);   // RC module
IPAddress gwAPip(192, 168, 1, 50);   // Gateway & Accesspoint IP
IPAddress mask(255, 255, 255, 0);
IPAddress myDNS(8, 8, 8, 8);       //optional

unsigned int portMy  = 5555; //this is port of this module: Section Control = 5555
unsigned int portRC = 3050; // port to listen for Android App Remote control

IPAddress myIP, myGW;
uint8_t myIP_ending = 50;//sets IP of ESP32 to x.x.x.myIP_ending

//IP address to send UDP data to:
IPAddress ipToAOG;
unsigned int portToAOG = 9999; // Port of AOG that listens


// IO pins --------------------------------
#define LED_BUILTIN		5  //LED auf dem Lolin D32 leuchtet auf LOW!!
#define BeLiAuf_PIN		23 //1  Kabel Nummer im Bedienpult
#define BeLiAb_PIN		22 //2
#define FwLiAuf_PIN		18 //9
#define FwLiAb_PIN		19 //10
#define LeRe_PIN		21 //5
#define LeLi_PIN		17 //6
#define FwReAuf_PIN		16 //7
#define BeReAuf_PIN		4  //3
#define BeReAb_PIN		0  //4
#define FwReAb_PIN		2  //8
#define WORKSW_PIN		33 //12


// Variables ------------------------------
  int WorkSWthreshold = 800;//Workswitch analog value. if higher than threshold than on
  bool WorkWS = false;
 
 //Array to send data back to AgOpenGPS
  byte WStoAOGon[] = { 127,249,0,0,0,0,1,0,0,0 };//section 1 = on
  byte WStoAOGoff[] = { 127,249,0,0,0,0,0,0,0,2 };//SC Auto = off

 //data that will be received from Android RC App
 int UDP_data[10] = { 0,0,0,0,0,0,0,0,0,0 }; 
 byte RC_data = 0, RC_data_old = 0, RC_data_lastMove = 0;

 // WiFi status LED blink times: searching WIFI: blinking 4x faster; connected: blinking as times set; data available: light on; no data for 1 second: blinking
  unsigned long LED_WIFI_time = 0;
  unsigned long LED_WIFI_pulse = 700;   //light on in ms 
  unsigned long LED_WIFI_pause = 500;   //light off in ms
  boolean LED_WIFI_ON = false;
  unsigned long UDP_data_time = 0;
  unsigned long UDP_watchdog_time = 500; //max time for no new UDP while relay is on
  unsigned long relay_start_time = 0;
  unsigned long relay_max_time = 4000; //max time for turning on relay for Behälter x4 times
  
 //loop time variables in microseconds
  const unsigned long LOOP_TIME = 500; //2Hz Frequenzy for sending work switch data
  unsigned long lastTime = LOOP_TIME;
  unsigned long currentTime = LOOP_TIME;
  byte watchdogTimer = 0;

//libraries -------------------------------
#include "Network_AOG.h"
#include "EEPROM.h"
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFi.h>


TaskHandle_t Core1;
TaskHandle_t Core2;

//instances
WiFiServer server(80);
WiFiClient client;
WiFiUDP udpListenToRC;
WiFiUDP udpSendToAOG;


// Setup procedure ------------------------
void setup() {
	delay(20);
	Serial.begin(115200);

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(BeLiAuf_PIN, OUTPUT);
	pinMode(BeLiAb_PIN, OUTPUT);
	pinMode(FwLiAuf_PIN, OUTPUT);
	pinMode(FwLiAb_PIN, OUTPUT);
	pinMode(LeRe_PIN, OUTPUT);
	pinMode(LeLi_PIN, OUTPUT);
	pinMode(FwReAuf_PIN, OUTPUT);
	pinMode(FwReAb_PIN, OUTPUT);
	pinMode(BeReAuf_PIN, OUTPUT);
	pinMode(BeReAb_PIN, OUTPUT);
	pinMode(WORKSW_PIN, INPUT_PULLUP);

	delay(2);
	digitalWrite(BeLiAuf_PIN, LOW);
	digitalWrite(BeLiAb_PIN, LOW);
	digitalWrite(FwLiAuf_PIN, LOW);
	digitalWrite(FwLiAb_PIN, LOW);
	digitalWrite(LeRe_PIN, LOW);
	digitalWrite(LeLi_PIN, LOW);
	digitalWrite(BeReAuf_PIN, LOW);
	digitalWrite(BeReAb_PIN, LOW);
	digitalWrite(FwReAuf_PIN, LOW);
	digitalWrite(FwReAb_PIN, LOW);

	if (RelayTest) {
		delay(2000);
		Serial.println("starting relay test: turn on one by one for 0.5s");
		digitalWrite(BeLiAuf_PIN, HIGH); delay(500);	digitalWrite(BeLiAuf_PIN, LOW); delay(1000);
		digitalWrite(BeLiAb_PIN, HIGH); delay(500);	digitalWrite(BeLiAb_PIN, LOW); delay(1000);
		digitalWrite(FwLiAuf_PIN, HIGH); delay(500);	digitalWrite(FwLiAuf_PIN, LOW); delay(1000);
		digitalWrite(FwLiAb_PIN, HIGH); delay(500);	digitalWrite(FwLiAb_PIN, LOW); delay(1000);
		digitalWrite(LeRe_PIN, HIGH); delay(500);	digitalWrite(LeRe_PIN, LOW); delay(1000);
		digitalWrite(LeLi_PIN, HIGH); delay(500);	digitalWrite(LeLi_PIN, LOW); delay(1000);
		digitalWrite(BeReAuf_PIN, HIGH); delay(500);	digitalWrite(BeReAuf_PIN, LOW); delay(1000);
		digitalWrite(BeReAb_PIN, HIGH); delay(500);	digitalWrite(BeReAb_PIN, LOW); delay(1000);
		digitalWrite(FwReAuf_PIN, HIGH); delay(500);	digitalWrite(FwReAuf_PIN, LOW); delay(1000);
		digitalWrite(FwReAb_PIN, HIGH); delay(500);	digitalWrite(FwReAb_PIN, LOW); 
		Serial.println("relay test done");
		delay(2000);
	}

	restoreEEprom();
	delay(20);
  
  //------------------------------------------------------------------------------------------------------------  
  //create a task that will be executed in the Core1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(Core1code, "Core1", 10000, NULL, 1, &Core1, 0);
  delay(500); 
  //create a task that will be executed in the Core2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(Core2code, "Core2", 10000, NULL, 1, &Core2, 1); 
  delay(500); 
  //------------------------------------------------------------------------------------------------------------

}
  



void loop() {
  
}
