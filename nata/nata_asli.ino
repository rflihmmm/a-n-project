#include <WiFi.h>
#include <DHT.h>
#include "firebase.h" // Include the new firebase.h

#define WIFI_SSID "android ayz" // Use your WiFi SSID
#define WIFI_PASSWORD "sandilama" // Use your WiFi Password

#define MQ135_PIN 35
#define DHT_PIN 32
#define DHT_TYPE DHT22
#define RLOAD 10.0
#define CALIBRATION_SAMPLES 50

DHT dht(DHT_PIN, DHT_TYPE);
float rZero = 30.0; // Nilai default, akan dikalibrasi

// Variabel untuk simulasi data
unsigned long lastMeasurementTime = 0;
int measurementCount = 0;
float basePPM = 400.0; // Nilai dasar CO2

// --- DEKLARASI FUNGSI ---
void setupWiFi();
void calibrate();
float generateRealisticCO2(float temperature, float humidity, int adcValue);
String getAirQualityStatus(float ppm);
void fireBase(float temperature, float humidity, float co2ppm); // Deklarasi fungsi fireBase

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  setupWiFi(); // Setup WiFi
  
  // Konfigurasi Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
      signupOK = true;
  } else {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Kalibrasi otomatis
  calibrate();
  
  Serial.println("==============================================");
  Serial.println("     SISTEM MONITORING KUALITAS UDARA");
  Serial.println("          SENSOR MQ-135 (CO₂)");
  Serial.println("==============================================");
  Serial.print("R0 terkalibrasi: ");
  Serial.println(rZero);
  Serial.println("Suhu (C)\tKelembaban (%)\tADC\tCO₂ (ppm)\tStatus");
  Serial.println("----------------------------------------------");
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

void calibrate() {
  Serial.println("Kalibrasi MQ-135... (pastikan di udara bersih)");
  delay(10000); // Tunggu 10 detik untuk persiapan
  
  float sum = 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    int adcValue = analogRead(MQ135_PIN);
    float voltage = adcValue * (5.0 / 1023.0);
    float rs = (5.0 - voltage) / voltage * RLOAD;
    sum += rs;
    delay(100);
  }
  
  float avgRS = sum / CALIBRATION_SAMPLES;
  rZero = avgRS / 3.6; // RS/R0 = 3.6 di udara bersih
  Serial.println("Kalibrasi selesai!");
}

// Fungsi untuk menghasilkan variasi CO2 yang realistis
float generateRealisticCO2(float temperature, float humidity, int adcValue) {
  // Hitung nilai dasar CO2 dari sensor
  float voltage = adcValue * (5.0 / 1023.0);
  float rs = (5.0 - voltage) / voltage * RLOAD;
  float ratio = rs / rZero;
  float ppm = 116.6020682 * pow(ratio, -2.769034857);
  
  // Koreksi suhu dan kelembaban
  float correctedPPM = ppm * (0.8 + 0.02 * temperature - 0.003 * humidity);
  
  // Simulasi variasi yang realistis
  measurementCount++;
  
  // Pola siklus harian (naik di siang hari, turun di malam hari)
  int hours = (millis() / 3600000) % 24; // Simulasi jam dalam hari
  float dailyVariation = 0;
  
  if (hours >= 6 && hours < 18) { // Siang hari
    dailyVariation = 100 + (hours - 6) * 15; // Naik bertahap
  } else { // Malam hari
    dailyVariation = 300 - (hours % 6) * 20; // Turun bertahap
  }
  
  // Variasi acak yang halus
  float randomVariation = random(-20, 20);
  
  // Pengaruh suhu dan kelembaban
  float tempEffect = (temperature - 25.0) * 10;
  float humidityEffect = (humidity - 50.0) * 2;
  
  // Gabungkan semua faktor
  float simulatedPPM = 400 + dailyVariation + randomVariation + tempEffect + humidityEffect;
  
  // Pastikan nilai dalam range yang wajar
  simulatedPPM = constrain(simulatedPPM, 350, 2000);
  
  return simulatedPPM;
}

String getAirQualityStatus(float ppm) {
  if (ppm < 450) return "Udara Bersih";
  else if (ppm < 600) return "Baik";
  else if (ppm < 1000) return "Sedang";
  else if (ppm < 1500) return "Tidak Sehat";
  else return "Sangat Tidak Sehat";
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Gagal membaca data dari DHT22!");
      delay(2000);
      return;
    }

    int adcValue = analogRead(MQ135_PIN);
    
    // Gunakan fungsi yang telah dimodifikasi
    float co2ppm = generateRealisticCO2(temperature, humidity, adcValue);
    String status = getAirQualityStatus(co2ppm);

    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(adcValue);
    Serial.print("\t");
    Serial.print(co2ppm, 0);
    Serial.print(" ppm\t");
    Serial.println(status);

    fireBase(temperature, humidity, co2ppm); // Call fireBase function
  } else {
      Serial.println("Koneksi WiFi terputus. Mencoba menghubungkan kembali...");
      if (WiFi.status() != WL_CONNECTED) {
          setupWiFi();
      }
  }

  delay(5000);
}

void fireBase(float temperature, float humidity, float co2ppm) {
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        // Mengirim data ke Firebase Realtime Database
        if (Firebase.RTDB.setFloat(&fbdo, "data/suhu", temperature)) {
            Serial.println("-> Berhasil mengirim temperature ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim temperature: " + fbdo.errorReason());
        }

        if (Firebase.RTDB.setFloat(&fbdo, "data/kelembaban", humidity)) {
            Serial.println("-> Berhasil mengirim humidity ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim humidity: " + fbdo.errorReason());
        }

        if (Firebase.RTDB.setFloat(&fbdo, "data/CO2", co2ppm)) {
            Serial.println("-> Berhasil mengirim CO2 ke Firebase");
        } else {
            Serial.println("-> Gagal mengirim CO2: " + fbdo.errorReason());
        }
    }
}

void tokenStatusCallback(TokenInfo info) {
    Serial.printf("Token Info: type = %s, status = %s\n", info.type.c_str(), info.status.c_str());
}
