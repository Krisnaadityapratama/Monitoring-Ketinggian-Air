#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Network credentials
#define WIFI_SSID "F1"
#define WIFI_PASSWORD "1234567899"

// Firebase config krisna
//#define FIREBASE_HOST "https://ketinggian-air-551c0-default-rtdb.firebaseio.com/"
//#define FIREBASE_AUTH "AIzaSyA0c2F8uR1t4PVsQNoTyd27_DfuNnwoSsk" 

// Firebase config aurell
 #define FIREBASE_HOST "https://ketinggian-air-sungai-belik-default-rtdb.firebaseio.com/"
 #define FIREBASE_AUTH "AIzaSyB7rNwfUKcAXbyXcCtjLLmDrrto4-3mJjM" 

// Firebase and WiFi objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Ultrasonic sensor pins
#define PIN_TRIG 12
#define PIN_ECHO 14

long durasi;
float cm;

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

  delay(60000);
}
