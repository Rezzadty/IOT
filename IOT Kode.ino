#include <ESP8266WiFi.h>           // Library untuk koneksi WiFi ESP8266
#include <LiquidCrystal_PCF8574.h> // Library untuk kontrol LCD I2C
#include <DHT.h>                   // Library untuk sensor suhu dan kelembapan DHT
#include <FirebaseESP8266.h>       // Library untuk koneksi Firebase
#include <time.h>                  // Library untuk sinkronisasi waktu NTP

// Konfigurasi WiFi
const char* ssid = "Warung Bekicot";          // Nama jaringan WiFi
const char* password = "warungbekicot";   // Password jaringan WiFi

// Konfigurasi Firebase
#define DATABASE_URL "https://iot-dc748-default-rtdb.asia-southeast1.firebasedatabase.app"
#define API_KEY "AIzaSyAIVD0ZqZ42wbVOX1nEFbGjso3m74GQVAQ"

FirebaseData firebaseData;            // Objek untuk komunikasi dengan Firebase
FirebaseJson json;                    // Objek JSON untuk menyimpan data
FirebaseConfig firebaseConfig;        // Konfigurasi Firebase
FirebaseAuth firebaseAuth;            // Informasi otorisasi Firebase

// Konfigurasi perangkat keras
#define DHTPIN D2                    // Pin untuk sensor DHT11
#define SOIL_MOISTURE_PIN A0         // Pin untuk sensor kelembapan tanah
#define RELAY_PIN D4                 // Pin untuk relay (pompa)
#define BATTERY_PIN A0               // Pin untuk membaca tegangan baterai
#define DHTTYPE DHT11                // Tipe sensor DHT yang digunakan

// Inisialisasi objek sensor dan LCD
DHT dht(DHTPIN, DHTTYPE);            // Inisialisasi objek DHT
LiquidCrystal_PCF8574 lcd(0x27);     // Inisialisasi LCD dengan alamat I2C

// Variabel untuk menyimpan data
int soilMoistureValue = 0;          // Nilai kelembapan tanah dari sensor (ADC)
float humidity = 0.0, temperature = 0.0; // Data suhu dan kelembapan udara
float batteryVoltage = 0.0;         // Tegangan baterai
bool manualControl = false;         // Mode kontrol manual pompa
bool pumpStatus = false;            // Status pompa (ON/OFF)

// Batasan dan ambang nilai
const float BATTERY_THRESHOLD = 3.0;       // Tegangan minimum untuk menyalakan pompa
#define RELAY_ACTIVE_LOW false             // Relay aktif rendah
#define SOIL_MOISTURE_THRESHOLD 40         // Ambang kelembapan tanah

// Interval untuk pembaruan data
unsigned long previousMillisLCD = 0;
unsigned long previousMillisFirebase = 0;
unsigned long previousMillisSensorData = 0;
const long intervalLCD = 1000;             // Interval pembaruan LCD (1 detik)
const long intervalFirebase = 5000;        // Interval pengiriman ke Firebase (5 detik)
const long intervalSensorData = 60000;     // Interval pengiriman data SensorData (1 menit)

// Konfigurasi NTP untuk sinkronisasi waktu
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // Offset waktu GMT+7 untuk WIB
const int daylightOffset_sec = 0;    // Tidak ada daylight saving

// Fungsi untuk menyalakan pompa
void pumpOn() {
  if (batteryVoltage >= BATTERY_THRESHOLD) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    pumpStatus = true;
    Serial.println("Pompa ON");
  } else {
    Serial.println("Pompa tidak dapat dinyalakan, baterai rendah!");
  }
}

// Fungsi untuk mematikan pompa
void pumpOff() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pumpStatus = false;
  Serial.println("Pompa OFF");
}

// Fungsi untuk mendapatkan waktu dalam format string
String getFormattedTime() {
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
  return String(buffer);
}

// Fungsi untuk mengirim data ke Firebase (RealTime)
void sendDataToFirebase(int soilMoisture, float temperature, float humidity, bool pumpState) {
  json.set("SoilMoisture", soilMoisture);
  json.set("Temperature", temperature);
  json.set("Humidity", humidity);
  json.set("PumpState", pumpState);
  json.set("Timestamp", getFormattedTime());

  if (Firebase.updateNode(firebaseData, "/RealTime", json)) {
    Serial.println("Data berhasil diperbarui di Firebase (Realtime)!");
  } else {
    Serial.print("Gagal memperbarui data di Firebase: ");
    Serial.println(firebaseData.errorReason());
  }
}

// Fungsi untuk menyimpan data ke Firebase (SensorData)
void sendDataToSensorData(int soilMoisture, float temperature, float humidity, bool pumpState) {
  String path = "/SensorData/" + getFormattedTime(); // Path berdasarkan waktu
  json.set("SoilMoisture", soilMoisture);
  json.set("Temperature", temperature);
  json.set("Humidity", humidity);
  json.set("PumpState", pumpState);

  if (Firebase.set(firebaseData, path, json)) {
    Serial.println("Data berhasil disimpan di Firebase (SensorData)!");
  } else {
    Serial.print("Gagal menyimpan data ke Firebase: ");
    Serial.println(firebaseData.errorReason());
  }
}

// Fungsi untuk memastikan koneksi WiFi tetap aktif
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus, mencoba menyambung...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

// Fungsi setup untuk inisialisasi
void setup() {
  Serial.begin(9600);                // Memulai komunikasi serial
  WiFi.begin(ssid, password);        // Menyambungkan ke WiFi

  // Tunggu koneksi WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Sinkronisasi waktu dengan NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Konfigurasi Firebase
  firebaseConfig.api_key = API_KEY;
  firebaseConfig.database_url = DATABASE_URL;
  firebaseAuth.user.email = "bagasjr13@gmail.com";
  firebaseAuth.user.password = "qwer1234";

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  dht.begin();                    // Inisialisasi sensor DHT
  lcd.begin(16, 2);               // Inisialisasi LCD
  lcd.setBacklight(255);          // Nyalakan backlight LCD
  lcd.print("Smart Watering");    // Tampilkan teks awal
  delay(2000);
  lcd.clear();

  pinMode(RELAY_PIN, OUTPUT);     // Atur pin relay sebagai OUTPUT
  pumpOff();                      // Pastikan pompa mati di awal
}

// Fungsi loop untuk logika utama
void loop() {
  checkWiFiConnection();          // Periksa koneksi WiFi
  unsigned long currentMillis = millis();

  // Pembaruan LCD
  if (currentMillis - previousMillisLCD >= intervalLCD) {
    previousMillisLCD = currentMillis;

    soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);            // Baca kelembapan tanah
    int soilMoisturePercent = map(soilMoistureValue, 0, 1023, 0, 100); // Konversi ke persentase

    humidity = dht.readHumidity();         // Baca kelembapan udara
    temperature = dht.readTemperature();   // Baca suhu udara
    batteryVoltage = (analogRead(BATTERY_PIN) * (5.0 / 1023.0)) * 2; // Baca tegangan baterai

    // Tampilkan data pada LCD
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Gagal membaca dari sensor DHT!");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Soil: ");
      lcd.print(soilMoisturePercent);
      lcd.print(" %");
      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print(temperature);
      lcd.print("C H:");
      lcd.print(humidity);
      lcd.print("%");
    }

    // Logika kontrol pompa
    if (manualControl) {
      pumpOn();
    } else {
      if (soilMoisturePercent < SOIL_MOISTURE_THRESHOLD) {
        pumpOn();
      } else {
        pumpOff();
      }
    }
  }

  // Pengiriman data ke Firebase
  if (currentMillis - previousMillisFirebase >= intervalFirebase) {
    previousMillisFirebase = currentMillis;
    sendDataToFirebase(soilMoistureValue, temperature, humidity, pumpStatus);
  }

  // Pengiriman data ke SensorData
  if (currentMillis - previousMillisSensorData >= intervalSensorData) {
    previousMillisSensorData = currentMillis;
    sendDataToSensorData(soilMoistureValue, temperature, humidity, pumpStatus);
  }
}