//Core2: this task only serves the Webpage

void Core2code( void * pvParameters ){
  Serial.println();
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  
// Start WiFi Client
 while (!EE_done){  // wait for eeprom data
  delay(10);
 }
 WiFi_Start_STA();
 if (my_WiFi_Mode == 0) WiFi_Start_AP(); // if failed start AP
 delay(2000);  // start WiFi
 
 UDP_Start();  // start the UDP Client
 delay(2000);


  for(;;){
    WiFi_Traffic();//handles settings web page
    delay(10);  

  //no data for more than 1 sec = blink 930ms: App sends every second command 0, if no button pressed, so see minimal blinking, when app connected
  if (millis() > (UDP_data_time + 930L)) {
    if (!LED_WIFI_ON) {
      if (millis() > (LED_WIFI_time + LED_WIFI_pause)) {
        LED_WIFI_time = millis();
        LED_WIFI_ON = true;
		digitalWrite(LED_BUILTIN, LOW);
      }
    }
    if (LED_WIFI_ON) {
      if (millis() > (LED_WIFI_time + LED_WIFI_pulse)) {
        LED_WIFI_time = millis();
        LED_WIFI_ON = false;
		digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  }
  
  else {
	  digitalWrite(LED_BUILTIN, LOW);
  }
  
  }
}
