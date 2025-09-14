#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyCMPbcZyanhZme8ZS5CBw_6X21y947hKHg"
#define DATABASE_URL "https://leangleang-default-rtdb.firebaseio.com/"

#define WIFI_SSID "ebyy" // Use your WiFi SSID
#define WIFI_PASSWORD "belikuota" // Use your WiFi Password

#define MQ135_PIN 35
#define DHT_PIN 32
#define DHT_TYPE DHT22
#define RLOAD 10.0
#define CALIBRATION_SAMPLES 50

// --- OBJEK GLOBAL FIREBASE ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// --- DEKLARASI FUNGSI FIREBASE ---
void tokenStatusCallback(TokenInfo info);
