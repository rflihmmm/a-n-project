#include <Firebase_ESP_Client.h>

#define API_KEY "AIzaSyA3Kt2igI_dSVyZ-fLxhMV8_feuRUtdFlM"
#define DATABASE_URL "https://projek-ta-e12c8-default-rtdb.firebaseio.com"

// --- KREDENSIAL WIFI ---
#define WIFI_SSID "android ayz"
#define WIFI_PASSWORD "sandilama"

// --- DEFINISI PIN SENSOR ---
#define TRIG_PIN 5
#define ECHO_PIN 18
#define TURBIDITY_PIN 34
#define TDS_PIN 35
#define PH_PIN 32

// --- OBJEK GLOBAL FIREBASE ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
