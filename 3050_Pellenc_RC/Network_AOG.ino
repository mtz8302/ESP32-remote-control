//---------------------------------------------------------------------
void WiFi_Start_STA() {
  unsigned long timeout;

  WiFi.mode(WIFI_STA);   //  Workstation 

  WiFi.begin(PellencSettings.ssid, PellencSettings.password);
  timeout = millis() + (timeoutRouter * 1000);
  LED_WIFI_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
    delay(50);
    Serial.print(".");
    //WIFI LED blink in double time while connecting
    if (!LED_WIFI_ON) {
      if (millis() > (LED_WIFI_time + (LED_WIFI_pause >> 2))) {
        LED_WIFI_time = millis();
        LED_WIFI_ON = true;
		digitalWrite(LED_BUILTIN, LOW);
       }
     }
    if (LED_WIFI_ON) {
      if (millis() > (LED_WIFI_time + (LED_WIFI_pulse >> 2))) {
        LED_WIFI_time = millis();
        LED_WIFI_ON = false;
		digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  }  
  Serial.println();
  delay(50);
   if (WiFi.status() == WL_CONNECTED) 
   {  
	  delay(200); 
	  Serial.print("Connected IP - Address : ");
	  myIP = WiFi.localIP();
	  Serial.println(myIP);
	  myIP[3] = myIP_ending; //set ESP32 IP to x.x.x.myIP_ending
	  Serial.print("changing IP to: ");
	  Serial.println(myIP);
	  myGW = WiFi.gatewayIP();
	  if (!WiFi.config(myIP, myGW, mask, myDNS))
	   {
		Serial.println("STA Failed to configure");
	   }
	  delay(300);
	  server.begin();
	  delay(500);
	  Serial.print("WiFi Client successfully connected to : ");
	  Serial.println(PellencSettings.ssid);
	  Serial.print("Connected IP - Address : ");
	  myIP = WiFi.localIP();
	  Serial.println(myIP);
	  ipToAOG = myIP;
	  ipToAOG[3] = 255;	 
	  my_WiFi_Mode = WIFI_STA;
   } 
  else 
   {
		WiFi.mode(WIFI_OFF);
		Serial.println("WLAN-Client-Connection failed");
   }
  delay(20);
}

//---------------------------------------------------------------------
void WiFi_Start_AP() {
  delay(50);
  WiFi.mode(WIFI_AP);   // Accesspoint
  delay(50);
  WiFi.softAP(ssid_ap, password_ap);
  delay(50);
  while (!SYSTEM_EVENT_AP_START) // wait until AP has started
   {
    delay(100);
    Serial.print(".");
   }
  delay(200);
  WiFi.softAPConfig(gwAPip, gwAPip, mask);  // set fix IP for AP
  delay(500);  
  server.begin();
  delay(500);
  myIP = WiFi.softAPIP();
  Serial.print("Accesspoint started - Name : ");
  Serial.print(ssid_ap);
  Serial.print( "    IP address: ");
  Serial.println(myIP);
  ipToAOG = myIP;
  ipToAOG[3] = 255;  
  my_WiFi_Mode = WIFI_AP;
}

//---------------------------------------------------------------------
void UDP_Start()
{
	delay(50);
	if (udpListenToRC.begin(portRC) == 1)
	{	
		if (debugmode) {
			delay(50);
			Serial.print("UDP Listening on IP: ");
			Serial.println(WiFi.localIP());
		}
	}
	else { Serial.println("error starting UDP server listening to Android RC"); }
	delay(50);
	if (udpSendToAOG.begin(portMy) == 1)
	{
		if (debugmode) {
			Serial.print("UDP writing from port: ");
			Serial.println(portMy);
		}
	}
	else { Serial.println("error starting UDP client writing to AOG"); }
	delay(10);
}

//---------------------------------------------------------------------
void WiFi_Traffic() {

  char my_char;
  int htmlPtr = 0;
  int myIdx;
  int myIndex;
  unsigned long my_timeout;
  

  // Check if a client has connected
  client = server.available();
  
  if (!client)  return;

  Serial.println("New Client.");           // print a message out the serial port
  
  my_timeout = millis() + 250L;
  while (!client.available() && (millis() < my_timeout) ) delay(10);
  delay(10);
  if (millis() > my_timeout)  
    {
      Serial.println("Client connection timeout!");
      return;
    }
  //---------------------------------------------------------------------
  //htmlPtr = 0;
  char c;
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
           
            make_HTML01();  // create Page array
           //---------------------------------------------------------------------
           // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
           // and a content-type so the client knows what's coming, then a blank line:
           strcpy(HTTP_Header , "HTTP/1.1 200 OK\r\n");
           strcat(HTTP_Header, "Content-Length: ");
           strcati(HTTP_Header, strlen(HTML_String));
           strcat(HTTP_Header, "\r\n");
           strcat(HTTP_Header, "Content-Type: text/html\r\n");
           strcat(HTTP_Header, "Connection: close\r\n");
           strcat(HTTP_Header, "\r\n");

           client.print(HTTP_Header);
           delay(20);
           send_HTML();
           
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') 
           { // if you got anything else but a carriage return character,
             currentLine += c;      // add it to the end of the currentLine
             if (currentLine.endsWith("HTTP")) 
               {
                if (currentLine.startsWith("GET ")) 
                 {
                  currentLine.toCharArray(HTML_String,currentLine.length());
                  Serial.println();
                  exhibit ("Request : ", HTML_String);
                  process_Request();
                 }  
               }
           }//end else
      } //end client available
    } //end while client.connected
    // close the connection:
    client.stop();
    Serial.print("Pagelength : ");
    Serial.print(strlen(HTML_String));
    Serial.println("   --> Client Disconnected.");
 }// end if client 
}
//---------------------------------------------------------------------
// Process given values
//---------------------------------------------------------------------
void process_Request()
{ int myIndex;

  if (Find_Start ("/?", HTML_String) < 0 && Find_Start ("GET / HTTP", HTML_String) < 0 )
    {
      //nothing to process
      return;
    }
  action = Pick_Parameter_Zahl("ACTION=", HTML_String);

  // WiFi access data
  if ( action == ACTION_SET_SSID) {

    myIndex = Find_End("SSID_MY=", HTML_String);
    if (myIndex >= 0) {
      for (int i=0;i<24;i++) PellencSettings.ssid[i]=0x00;
      Pick_Text(PellencSettings.ssid, &HTML_String[myIndex], 24);
      exhibit ("SSID  : ", PellencSettings.ssid);
    }

    myIndex = Find_End("Password_MY=", HTML_String);
    if (myIndex >= 0) {
      for (int i=0;i<24;i++) PellencSettings.password[i]=0x00;
      Pick_Text(PellencSettings.password, &HTML_String[myIndex], 24);
      exhibit ("Password  : ", PellencSettings.password);
      EEprom_write_all();
    }
  }
}  
   
//---------------------------------------------------------------------
// HTML Seite 01 aufbauen
//---------------------------------------------------------------------
void make_HTML01() {

  strcpy( HTML_String, "<!DOCTYPE html>");
  strcat( HTML_String, "<html>");
  strcat( HTML_String, "<head>");
  strcat( HTML_String, "<title>Pellenc 3050 Fernsteuerung ESP32 Config Page</title>");
  strcat( HTML_String, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0;\" />\r\n");
  strcat( HTML_String, "<style>divbox {background-color: lightgrey;width: 200px;border: 5px solid red;padding:10px;margin: 10px;}</style>");
  strcat( HTML_String, "</head>");
  strcat( HTML_String, "<body bgcolor=\"#66b3ff\">");
  strcat( HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
  strcat( HTML_String, "<h1>Pellenc 3050 Fernsteuerung</h1>");

  //-----------------------------------------------------------------------------------------
  // WiFi Client Access Data
  strcat( HTML_String, "(by MTZ8302, webpage by WEder)<br>");
  strcat( HTML_String, "<hr><h2>WiFi Network Client Access Data</h2>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "If access fails, an accesspoint will be created<br>");
  strcat( HTML_String, "(Pellenc_3050_AP: open/no password)<br><br>");
  strcat( HTML_String, "<table>");
  set_colgroup(200, 300, 150, 0, 0);

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Network Name</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"SSID_MY\" maxlength=\"22\" Value =\"");
  strcat( HTML_String, PellencSettings.ssid);
  strcat( HTML_String, "\"></td>");
  
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_SSID);
  strcat( HTML_String, "\">Submit</button></td>");
  strcat( HTML_String, "</tr>");

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Password</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"Password_MY\" maxlength=\"22\" Value =\"");
  strcat( HTML_String, PellencSettings.password);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");
   

  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "</font>");
  strcat( HTML_String, "</font>");
  strcat( HTML_String, "</body>");
  strcat( HTML_String, "</html>");
}

//--------------------------------------------------------------------------
void send_not_found() {

  Serial.println("Send Not Found");

  client.print("HTTP/1.1 404 Not Found\r\n\r\n");
  delay(20);
  //client.stop();
}

//--------------------------------------------------------------------------
void send_HTML() {
  char my_char;
  int  my_len = strlen(HTML_String);
  int  my_ptr = 0;
  int  my_send = 0;

  //--------------------------------------------------------------------------
  // in Portionen senden
  while ((my_len - my_send) > 0) {
    my_send = my_ptr + MAX_PACKAGE_SIZE;
    if (my_send > my_len) {
      client.print(&HTML_String[my_ptr]);
      delay(20);

      //Serial.println(&HTML_String[my_ptr]);

      my_send = my_len;
    } else {
      my_char = HTML_String[my_send];
      // Auf Anfang eines Tags positionieren
      while ( my_char != '<') my_char = HTML_String[--my_send];
      HTML_String[my_send] = 0;
      client.print(&HTML_String[my_ptr]);
      delay(20);
      
      //Serial.println(&HTML_String[my_ptr]);

      HTML_String[my_send] =  my_char;
      my_ptr = my_send;
    }
  }
  //client.stop();
}

//----------------------------------------------------------------------------------------------
void set_colgroup(int w1, int w2, int w3, int w4, int w5) {
  strcat( HTML_String, "<colgroup>");
  set_colgroup1(w1);
  set_colgroup1(w2);
  set_colgroup1(w3);
  set_colgroup1(w4);
  set_colgroup1(w5);
  strcat( HTML_String, "</colgroup>");

}
//------------------------------------------------------------------------------------------
void set_colgroup1(int ww) {
  if (ww == 0) return;
  strcat( HTML_String, "<col width=\"");
  strcati( HTML_String, ww);
  strcat( HTML_String, "\">");
}


//---------------------------------------------------------------------
void strcatf(char* tx, float f) {
  char tmp[8];

  dtostrf(f, 6, 2, tmp);
  strcat (tx, tmp);
}
//---------------------------------------------------------------------
void strcati(char* tx, int i) {
  char tmp[8];

  itoa(i, tmp, 10);
  strcat (tx, tmp);
}

//---------------------------------------------------------------------
void strcati2(char* tx, int i) {
  char tmp[8];

  itoa(i, tmp, 10);
  if (strlen(tmp) < 2) strcat (tx, "0");
  strcat (tx, tmp);
}

//---------------------------------------------------------------------
int Pick_Parameter_Zahl(const char * par, char * str) {
  int myIdx = Find_End(par, str);

  if (myIdx >= 0) return  Pick_Dec(str, myIdx);
  else return -1;
}
//---------------------------------------------------------------------
int Find_End(const char * such, const char * str) {
  int tmp = Find_Start(such, str);
  if (tmp >= 0)tmp += strlen(such);
  return tmp;
}

//---------------------------------------------------------------------
int Find_Start(const char * such, const char * str) {
  int tmp = -1;
  int ww = strlen(str) - strlen(such);
  int ll = strlen(such);

  for (int i = 0; i <= ww && tmp == -1; i++) {
    if (strncmp(such, &str[i], ll) == 0) tmp = i;
  }
  return tmp;
}
//---------------------------------------------------------------------
int Pick_Dec(const char * tx, int idx ) {
  int tmp = 0;

  for (int p = idx; p < idx + 5 && (tx[p] >= '0' && tx[p] <= '9') ; p++) {
    tmp = 10 * tmp + tx[p] - '0';
  }
  return tmp;
}
//----------------------------------------------------------------------------
int Pick_N_Zahl(const char * tx, char separator, byte n) {

  int ll = strlen(tx);
  int tmp = -1;
  byte anz = 1;
  byte i = 0;
  while (i < ll && anz < n) {
    if (tx[i] == separator)anz++;
    i++;
  }
  if (i < ll) return Pick_Dec(tx, i);
  else return -1;
}

//---------------------------------------------------------------------
int Pick_Hex(const char * tx, int idx ) {
  int tmp = 0;

  for (int p = idx; p < idx + 5 && ( (tx[p] >= '0' && tx[p] <= '9') || (tx[p] >= 'A' && tx[p] <= 'F')) ; p++) {
    if (tx[p] <= '9')tmp = 16 * tmp + tx[p] - '0';
    else tmp = 16 * tmp + tx[p] - 55;
  }

  return tmp;
}

//---------------------------------------------------------------------
void Pick_Text(char * tx_ziel, char  * tx_quelle, int max_ziel) {

  int p_ziel = 0;
  int p_quelle = 0;
  int len_quelle = strlen(tx_quelle);

  while (p_ziel < max_ziel && p_quelle < len_quelle && tx_quelle[p_quelle] && tx_quelle[p_quelle] != ' ' && tx_quelle[p_quelle] !=  '&') {
    if (tx_quelle[p_quelle] == '%') {
      tx_ziel[p_ziel] = (HexChar_to_NumChar( tx_quelle[p_quelle + 1]) << 4) + HexChar_to_NumChar(tx_quelle[p_quelle + 2]);
      p_quelle += 2;
    } else if (tx_quelle[p_quelle] == '+') {
      tx_ziel[p_ziel] = ' ';
    }
    else {
      tx_ziel[p_ziel] = tx_quelle[p_quelle];
    }
    p_ziel++;
    p_quelle++;
  }

  tx_ziel[p_ziel] = 0;
}
//---------------------------------------------------------------------
char HexChar_to_NumChar( char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 55;
  return 0;
}
//---------------------------------------------------------------------
void exhibit(const char * tx, int v) {
  Serial.print(tx);
  Serial.println(v);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, unsigned int v) {
  Serial.print(tx);
  Serial.println(v);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, unsigned long v) {
  Serial.print(tx);
  Serial.println(v);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, const char * v) {
  Serial.print(tx);
  Serial.println(v);
}
