//Core1:  UDP Traffic Code

void Core1code( void * pvParameters ){
  
  int AnalogValue = 0;
	
  Serial.println();
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());


while ((my_WiFi_Mode == 0)){   // Waiting for WiFi Access
   //Serial.print(my_WiFi_Mode);
   Serial.println(" Waiting for WiFi Access\n");
   delay(4000);
 }
if (my_WiFi_Mode == WIFI_STA) Serial.println("connection to WiFi Network established");
if (my_WiFi_Mode == WIFI_AP)  Serial.println("WiFi Accesspoint now started");
delay(5000);//UDP start by core 2 code


  for(;;){ // MAIN LOOP FOR THIS CORE
	  
	  udpRCRecv(); //udp Message send by android app?
	  currentTime = millis();
	  //changes?
	  if (RC_data != RC_data_old) {		
		 if (RC_data == 0) { RC_data_lastMove = RC_data_old; }
		 else {
			 if (RC_data != RC_data_lastMove) { relay_start_time = currentTime; }//other command?
		 }
		 RC_data_old = RC_data;
	  }
	  //if UDP command received
	  if (RC_data > 0) {
		  //safety stopp
		  if (RC_data > 4) {
			  if ((relay_start_time + relay_max_time) < currentTime) {
				  Serial.println("Relay stopped: too long active");
				  RC_data = 0;
			  }
		  }
		  else {//Behälter: längere Zeit aktiv
			  if ((relay_start_time + (relay_max_time*4)) < currentTime) {
				  Serial.println("Relay stopped: too long active");
				  RC_data = 0;
			  }
		  }
		  if ((UDP_data_time + UDP_watchdog_time) < currentTime) {
			  Serial.print("Relay stopped: no new UDP for ms: "); Serial.println(currentTime - UDP_data_time);
			  RC_data = 0;
		  }
	  }
	  SetRelays();
	  delay(1);

	  //* Loop triggers every 500 msec and sends back workswitch
	  currentTime = millis();
 
	  if (currentTime - lastTime >= LOOP_TIME)
	  {
		lastTime = currentTime;
	
		AnalogValue = analogRead(WORKSW_PIN);
		delay(1);
		AnalogValue += analogRead(WORKSW_PIN);
		delay(1);
		AnalogValue += analogRead(WORKSW_PIN);
		delay(1);
		AnalogValue += analogRead(WORKSW_PIN);
		AnalogValue = AnalogValue >> 2;
		if (AnalogValue > WorkSWthreshold) {
			if (udpSendToAOG.beginPacket(ipToAOG, portToAOG) == 0) {
				Serial.print("error opening UDP to AOG  IP: ");
				Serial.print(ipToAOG);
				Serial.print("  Port: ");
				Serial.println(portToAOG);
			}
			else {
				//UDP packet start OK
				udpSendToAOG.write(WStoAOGon, 10);
				if (!WorkWS) {
					Serial.println("Workswitch ON");
					WorkWS = true;
				}
				if (udpSendToAOG.endPacket() == 0) {
					Serial.println("error sending UDP to AOG");
				}
			}
		}
		else
		{
			if (udpSendToAOG.beginPacket(ipToAOG, portToAOG) == 0) {
				Serial.print("error opening UDP to AOG  IP: ");
				Serial.print(ipToAOG);
				Serial.print("  Port: ");
				Serial.println(portToAOG);
			}
			else {
				//UDP packet start OK
				if (debugmode) {
					Serial.print("opening UDP to AOG  IP: ");
					Serial.print(ipToAOG);
					Serial.print("  Port: ");
					Serial.println(portToAOG);
				}
				udpSendToAOG.write(WStoAOGoff, 10);
				if (WorkWS) {
					Serial.println("Workswitch OFF");
					WorkWS = false;
				}
				if (udpSendToAOG.endPacket() == 0) {
					Serial.println("error sending UDP to AOG");
				}
			}
		}//end workswitch

	  }  // End of timed loop ------ 

} // End of (main core1)
} // End of core1code


// Subs --------------------------------------
void udpRCRecv()
{ //callback when received packets
	if (udpListenToRC.parsePacket() > 0)
		//packet came in
		{
		if (debugmode) { Serial.print("UDP comming in "); }

		for (int i = 0; udpListenToRC.available() > 0; i++)
			{
				UDP_data[i] = udpListenToRC.read();
				if (debugmode) { Serial.print(UDP_data[i]); }
				if (i > 5) {
					//only first 3 bytes needed
					udpListenToRC.flush();
					delay(1);
					break;
				}
			}
			if (debugmode) { Serial.println(); }
			//"RC"+data
			if ((UDP_data[0] == 82) & (UDP_data[1] == 67)) {
				UDP_data_time = millis();
				if (UDP_data[2] == 65) {//"A"=10
					RC_data = 10;
					if (debugmode) { Serial.print("command found: "); Serial.println(RC_data); }
				}
				else
				{
					RC_data = byte(UDP_data[2] - 48);//ASCII convertion
					if (debugmode) { Serial.print("command found: "); Serial.println(RC_data); }
				} 			
			}
	}  // end if new data
}
