#include "Variable.h"
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
// inisiasi RELAY
pinMode(r1,OUTPUT);  // RELAY 1
pinMode(r2,OUTPUT);  // RELAY 2
pinMode(r3,OUTPUT);  // RELAY 3
/// inisiasi LCD
lcd.init();
lcd.begin(16,2);
lcd.backlight();
lcd.setCursor(2, 0);
lcd.print("TUGAS AKHIR");
delay(2000);
lcd.clear();

pinMode(ph_pin,INPUT); // inisiasi pin PH sensor
pinMode(pin_k,INPUT);  // inisiasi pin Tubidity sensor

// inisiasi WIFI
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  /// inisiasi FIREBASE
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // put your main code here, to run repeatedly:
////////////////////// data PH ////////////////
data_ph = ph_sensor();
sprintf(buf,"pH : %.2f ",Po);
lcd.setCursor(0,0);
lcd.print(buf); // tampil LCD PH
//////////////////////////////////////////////

//////////////////// data KEKERUHAN ///////////
lcd.setCursor(0,1);
lcd.print("Tur : ");
value_kekeruhan = analogRead(pin_k);
data = map(value_kekeruhan,0,4096,100,0);  
sprintf(bufer,"%.3d",data); // tampil LCD Kekeruhan
lcd.print(bufer);
////////////////////////////////////////////////
fireBase(); /// fungsi kirim data ke Firebase

if(data<13){ 
  lcd.setCursor(11,1);
  lcd.print("JERNIH");
}
if(data>14<69){
  lcd.setCursor(11,1);
  lcd.print("KERUH");
}
if(data>70){
  lcd.setCursor(11,1);
  lcd.print("KOTOR");
}
delay(1000);
///////////////////////////////// OTOMATIS PROGRAM ////////////////
if(Po==7){ 
digitalWrite(r1,HIGH);
}
else if(Po < 7){
digitalWrite(r1,LOW);
digitalWrite(r2,LOW);
digitalWrite(r3,LOW);
}

digitalWrite(r1,button1);
digitalWrite(r2,button2);
digitalWrite(r3,button3);  
////////////////////////////////////////////////////////////////////
}
void fireBase(){
////////////////////////////////// kirim dan terima data FireBase ////////////////////////
if (Firebase.RTDB.setFloat(&fbdo, "value/ph", Po)){
//      Serial.println("PASSED");
//      Serial.println("PATH: " + fbdo.dataPath());
//      Serial.println("TYPE: " + fbdo.dataType());
//      Serial.println("ETag: " + fbdo.ETag());
    }
    else {
//      Serial.println("FAILED");
//      Serial.println("REASON: " + fbdo.errorReason());
    }
if (Firebase.RTDB.setFloat(&fbdo, "value/turbidity", data)){
//      Serial.println("PASSED");
//      Serial.println("PATH: " + fbdo.dataPath());
//      Serial.println("TYPE: " + fbdo.dataType());
//      Serial.println("ETag: " + fbdo.ETag());
    }
    else {
//      Serial.println("FAILED");
//      Serial.println("REASON: " + fbdo.errorReason());
    }
  if (Firebase.RTDB.getString(&fbdo, "/value/button1")) {
      if (fbdo.dataType() == "string") {
        button1 = fbdo.stringData().toInt();
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
  if (Firebase.RTDB.getString(&fbdo, "/value/button2")) {
      if (fbdo.dataType() == "string") {
        button2 = fbdo.stringData().toInt();
        
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
  if (Firebase.RTDB.getString(&fbdo, "/value/button3")) {
      if (fbdo.dataType() == "string") {
        button3 = fbdo.stringData().toInt();
        
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
///////////////////////////////////////////// END FOR FIREBASE /////////////////////////////////////
}

float ph_sensor(){ //////// fungsi data PH
ph_value = analogRead(ph_pin);
voltage = ph_value * (5/4096.0);
Serial.println(voltage,3);
value = (PH4 - PH7) / 3;
Po = 7.00+((PH7 - voltage) / value);
Serial.println(Po,2);
return Po;
}

int turbidity(){ //// FUNGSI data Turbidity
   for ( int i=0; i<10; i++){
    int val = analogRead(pin_k);
    teg[i] = val * (3.3/4096);
  }
  rata_rata_teg = (teg[0] + teg[1] + teg[2] + teg[3] + teg[4] + teg[5] + teg[6] + teg[7] + teg[8] + teg[9])/10 ;
  tds = (211.2254 * rata_rata_teg) - 144.1466;
  if(tds<0){tds=0;}
return tds;
}
