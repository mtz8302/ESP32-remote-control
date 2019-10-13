void SetRelays(void)
 {
    if (RC_data==1) digitalWrite(BeLiAuf_PIN, HIGH);
    else digitalWrite(BeLiAuf_PIN, LOW);
	if (RC_data == 2) digitalWrite(BeLiAb_PIN, HIGH);
	else digitalWrite(BeLiAb_PIN, LOW);
	if (RC_data == 9) digitalWrite(FwLiAuf_PIN, HIGH);
	else digitalWrite(FwLiAuf_PIN, LOW);
	if (RC_data == 10) digitalWrite(FwLiAb_PIN, HIGH);
	else digitalWrite(FwLiAb_PIN, LOW);
	if (RC_data == 5) digitalWrite(LeRe_PIN, HIGH);
	else digitalWrite(LeRe_PIN, LOW);
	if (RC_data == 6) digitalWrite(LeLi_PIN, HIGH);
	else digitalWrite(LeLi_PIN, LOW);
	if (RC_data == 3) digitalWrite(BeReAuf_PIN, HIGH);
	else digitalWrite(BeReAuf_PIN, LOW);
	if (RC_data == 4) digitalWrite(BeReAb_PIN, HIGH);
	else digitalWrite(BeReAb_PIN, LOW);
	if (RC_data == 7) digitalWrite(FwReAuf_PIN, HIGH);
	else digitalWrite(FwReAuf_PIN, LOW);
	if (RC_data == 8) digitalWrite(FwReAb_PIN, HIGH);
	else digitalWrite(FwReAb_PIN, LOW);
}

//--------------------------------------------------------------
//  EEPROM Data Handling
//--------------------------------------------------------------
#define EEPROM_SIZE 128
#define EE_ident1 0xDE  // Marker Byte 0 + 1
#define EE_ident2 0xEA

//--------------------------------------------------------------
//  Restore EEprom Data
//--------------------------------------------------------------
void restoreEEprom(){
  //byte get_state  = digitalRead(restoreDefault_PIN);
  byte get_state = false;
  
  if (EEprom_empty_check()==1 || get_state || Reset_EEPROM_settings) { //first start?
    EEprom_write_all();     //write default data
   }
  if ((EEprom_empty_check()==2) && (!Reset_EEPROM_settings)) { //data available
    EEprom_read_all();
   }
  //EEprom_show_memory();  //
  EE_done =1;   
}

//--------------------------------------------------------------
byte EEprom_empty_check(){
    
  if (!EEPROM.begin(EEPROM_SIZE))  
    {
     Serial.println("failed to initialise EEPROM"); delay(1000);
     return false;
    }
  if (EEPROM.read(0)!= EE_ident1 || EEPROM.read(1)!= EE_ident2)
     return true;  // is empty
  
  if (EEPROM.read(0)== EE_ident1 && EEPROM.read(1)== EE_ident2)
     return 2;     // data available
     
 }
//--------------------------------------------------------------
void EEprom_write_all(){
  EEPROM.write(0, EE_ident1);
  EEPROM.write(1, EE_ident2);
  EEPROM.put(4, PellencSettings);
  EEPROM.commit();
}
//--------------------------------------------------------------
void EEprom_read_all(){
    EEPROM.get(4, PellencSettings);
}
//--------------------------------------------------------------
void EEprom_show_memory(){
byte c2=0, data_;
int len = sizeof(PellencSettings);
  Serial.print("Reading ");
  Serial.print(len);
  Serial.println(" bytes from Flash . Values are:");
  for (int i = 0; i < len; i++)
  { 
    data_=byte(EEPROM.read(i));
    if (data_ < 0x10) Serial.print("0");
    Serial.print(data_,HEX); 
    if (c2>=15) {
       Serial.println();
       c2=-1;
      }
    else Serial.print(" ");
    c2++;
  }
}




   
