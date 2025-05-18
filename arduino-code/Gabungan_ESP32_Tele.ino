#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HTTPClient.h>

// Network credentials
#define WIFI_SSID "Aurell_Arine"
#define WIFI_PASSWORD "aurellarine"

// Firebase config aurell
 #define FIREBASE_HOST "https://ketinggian-air-sungai-belik-default-rtdb.firebaseio.com/"
 #define FIREBASE_AUTH "AIzaSyB7rNwfUKcAXbyXcCtjLLmDrrto4-3mJjM" 

// Telegram config
String botToken = "7323920625:AAHieSajecgla0UCAqQ9GrT-shyIDypWBr8";
String chatID = "6456563480";

// Firebase and WiFi objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Ultrasonic sensor pins
#define PIN_TRIG 12
#define PIN_ECHO 14

long durasi;
float cm;
float ketinggianAir;
bool sudahKirimNotifikasi = false;
bool sudahKirimUpdate = false; 

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Set Firebase credentials
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  // Hitung ketinggian air
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  durasi = pulseIn(PIN_ECHO, HIGH);
  cm = (durasi * 0.0343) / 2;
  ketinggianAir = cm;

  Serial.print("Ketinggian air: ");
  Serial.print(cm);
  Serial.println(" cm");

  // Kirim data ke Firebase
  if (Firebase.RTDB.setFloat(&fbdo, "/Sensor1", cm)) {
    Serial.println("Data sent to Firebase");
  } else {
    Serial.print("Firebase set failed: ");
    Serial.println(fbdo.errorReason());
  }

  // ✅ Tambahan update real-time
  if (ketinggianAir > 100 && !sudahKirimUpdate) {
    String pesanUpdate = "📡 Ketinggian Air Sungai Belik\nSaat ini: " + String(ketinggianAir, 2) + " cm.";
    kirimPeringatan(pesanUpdate);
    sudahKirimUpdate = true;
  } else if (ketinggianAir <= 100) {
    sudahKirimUpdate = false; // reset kalau air turun
  }
    // ⚠️ Peringatan bahaya jika lebih dari 150 cm
  if (ketinggianAir > 150 && !sudahKirimNotifikasi) {
    kirimPeringatan("⚠️ Peringatan Banjir! Tinggi air mencapai " + String(ketinggianAir) + " cm.");
    sudahKirimNotifikasi = true;
  } else if (ketinggianAir <= 150) {
    sudahKirimNotifikasi = false; // reset kalau air udah turun
  }

  delay(60000);
}

void kirimPeringatan(String pesan) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + urlEncode(pesan);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("✅ Notifikasi berhasil dikirim!");
    } else {
      Serial.println("❌ Gagal kirim notifikasi.");
    }

    http.end();
  } else {
    Serial.println("WiFi tidak terkoneksi.");
  }
}

// Fungsi untuk URL Encode
String urlEncode(String str) {
  String encoded = "";
  char c;
  char bufHex[4];
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      sprintf(bufHex, "%%%02X", c);  // konversi ke HEX
      encoded += bufHex;
    }
  }
  return encoded;
}
