//////////////////////////// Library variable
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
////////////////////////////// database
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "hellboy"
#define WIFI_PASSWORD "87654321"

#define API_KEY "AIzaSyD4tfqP0jqoFjNcuzK6W892EEOf4fdKhSg"
#define DATABASE_URL "https://iotkadarair-default-rtdb.firebaseio.com/" 

/// variable firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
///////////////////////////// Sensor Ph
int ph_value;
float voltage;
#define ph_pin A7
float PH4 = 3.226;
float PH7 = 2.691;
float value;
float Po;
char buf[14];
char bufer[23];
///////////////////////////////// Sensor Kekeruhan
#define pin_k A6
int value_k ;
float rata_rata_teg;
int tds;
float teg[10];
/////////////////
int r1=32; int r2=33 ;int r3=26;
int button1,button2,button3;
int value_kekeruhan;
int data;
float data_ph;
