#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyCMPbcZyanhZme8ZS5CBw_6X21y947hKHg"
#define DATABASE_URL "https://leangleang-default-rtdb.firebaseio.com/"

// --- OBJEK GLOBAL FIREBASE ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// --- DEKLARASI FUNGSI FIREBASE ---
void tokenStatusCallback(TokenInfo info);
