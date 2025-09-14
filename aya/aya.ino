#include <WiFi.h>

// --- KREDENSIAL FIREBASE (dari file firebase.h) ---
#include "firebase.h"

// --- KONSTANTA ---
const float tankHeight = 30.0;
const float tankCapacity = 100.0;
const int pHCalibrationSamples = 20;
const float pHNeutralVoltage = 1.4975;
const float pHPerVolt = 3.5;

// --- DEKLARASI FUNGSI ---
float readDistanceCM();
float calculateWaterLevel(float distance);
float calculateVolume(float waterLevel);
float readTurbidity();
float readTurbidityAvg(int samples = 10);
float readTDS();
float readpH();
float readAvgVoltage(int pin, int samples);
void displayReadings(float level, float vol, float turb, float tds, float ph);
void setupWiFi();
void fireBase(float waterLevel, float turbidity, float tds, float pHValue); // Deklarasi fungsi fireBase

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    setupWiFi();

    // Konfigurasi Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if (Firebase.signUp(&config, &auth, "", "")){
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    // Menghubungkan ke Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("Sistem Monitoring Kualitas Air");
    Serial.println("============================");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) { // Removed Firebase.ready()
        float distance = readDistanceCM();
        float waterLevel = calculateWaterLevel(distance);
        float volume = calculateVolume(waterLevel);
        float turbidity = readTurbidityAvg();
        float tds = readTDS();
        float pHValue = readpH();

        displayReadings(waterLevel, volume, turbidity, tds, pHValue);

        fireBase(waterLevel, turbidity, tds, pHValue); // Call fireBase function

    } else {
        Serial.println("Koneksi WiFi terputus. Mencoba menghubungkan kembali...");
        if (WiFi.status() != WL_CONNECTED) {
            setupWiFi();
        }
    }

    delay(5000); // Menambah jeda untuk stabilitas koneksi
}

void setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Menghubungkan ke WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
}

float calculateWaterLevel(float distance) {
    return constrain(tankHeight - distance, 0, tankHeight);
}

float calculateVolume(float waterLevel) {
    return (waterLevel / tankHeight) * tankCapacity;
}

void displayReadings(float level, float vol, float turb, float tds, float ph) {
    Serial.println("\n=== HASIL PEMBACAAN ===");
    Serial.printf("Ketinggian Air: %.1f cm\n", level);
    Serial.printf("Volume Air: %.1f liter\n", vol);

    if (turb < 0)
        Serial.println("Kekeruhan: Sensor Error!");
    else
        Serial.printf("Kekeruhan: %.1f NTU\n", turb);

    Serial.printf("TDS: %.1f ppm\n", tds);
    Serial.printf("pH: %.2f\n", ph);
    Serial.println("=======================");
}

float readDistanceCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    return duration * 0.034 / 2; // Konversi ke cm
}

float readTurbidity() {
    int raw = analogRead(TURBIDITY_PIN);
    float voltage = raw * (3.3 / 4095.0);
    // - 3.0V = 0 NTU (air jernih)
    // - 1.0V = 1000 NTU (keruh)
    float ntu = (1000.0 / (1.0 - 3.0)) * (voltage - 3.0);
    return ntu;
}

float readTurbidityAvg(int samples) {
    float sum = 0;
    int validReadings = 0;
    for (int i = 0; i < samples; i++) {
        float turb = readTurbidity();
        if (turb >= 0) {
            sum += turb;
            validReadings++;
        }
        delay(50);
    }

    if (validReadings == 0) return -1.0; // Semua pembacaan error
    return sum / validReadings;
}

float readTDS() {
    int raw = analogRead(TDS_PIN);
    float voltage = raw * (3.3 / 4095.0);
    return (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage * 0.5);
}

float readpH() {
    float voltage = readAvgVoltage(PH_PIN, pHCalibrationSamples);
    float pHValue = 7.0 + ((pHNeutralVoltage - voltage) * pHPerVolt);
    return constrain(pHValue, 0, 14);
}

float readAvgVoltage(int pin, int samples) {
    float sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(10);
    }
    return (sum / samples) * (3.3 / 4095.0);
}

void fireBase(float waterLevel, float turbidity, float tds, float pHValue) {
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) { // Add Firebase.ready() check here
        sendDataPrevMillis = millis();

        // Mengirim data ke Firebase Realtime Database
        if (Firebase.RTDB.setFloat(&fbdo, "sensorData/waterLevel", waterLevel)) {
            Serial.println("-> Berhasil mengirim waterLevel ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim waterLevel: " + fbdo.errorReason());
        }

        if (Firebase.RTDB.setFloat(&fbdo, "sensorData/turbidity", turbidity)) {
            Serial.println("-> Berhasil mengirim turbidity ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim turbidity: " + fbdo.errorReason());
        }

        if (Firebase.RTDB.setFloat(&fbdo, "sensorData/tds", tds)) {
            Serial.println("-> Berhasil mengirim tds ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim tds: " + fbdo.errorReason());
        }

        if (Firebase.RTDB.setFloat(&fbdo, "sensorData/phValue", pHValue)) {
            Serial.println("-> Berhasil mengirim phValue ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim phValue: " + fbdo.errorReason());
        }
    }
}

