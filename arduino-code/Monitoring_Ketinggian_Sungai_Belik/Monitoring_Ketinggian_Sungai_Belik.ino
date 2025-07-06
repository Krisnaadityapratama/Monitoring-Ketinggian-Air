#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>  // Library for secure Wi-Fi connections
#include <UniversalTelegramBot.h>
#include <ArduinoHttpClient.h>

// WiFi Manager
WiFiManager wifiManager;
const int buttonPin = 19; // Tombol reset WiFi (GPIO 19)

// Firebase Config
const char FIREBASE_HOST[] = "ketinggian-air-sungai-belik-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "AIzaSyB7rNwfUKcAXbyXcCtjLLmDrrto4-3mJjM";
const int SSL_PORT = 443;

// Sensor Ultrasonik
const int trigPin = 5;
const int echoPin = 18;
int kedalamanSungai = 200; // cm

// Telegram Bot Config
const char* botToken = "7611350014:AAGeMt1JbERea90GC_V0BrXKGG2naY1phk4";  // Token bot yang diberikan
const char* chat_id = "-1002533311710";  // ID Group Telegram Anda

// Threshold ketinggian yang ingin dipantau
const int thresholdTinggiAir = 170;  // Dalam cm, misalnya jika tinggi air melebihi 100 cm, baru dikirim ke Telegram

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Mencoba koneksi WiFi...");
  if (!wifiManager.autoConnect("ESP32-Access-Point-1", "12345678")) {
    Serial.println("Gagal terhubung! Reset ESP.");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Wi-Fi Terkoneksi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Mengatur koneksi SSL untuk Telegram
  client.setInsecure();
}

void loop() {
  // Mengecek tombol untuk reset WiFi
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("Tombol ditekan! Reset WiFi...");
    WiFi.disconnect();  // Putuskan koneksi WiFi
    wifiManager.resetSettings();  // Reset pengaturan WiFi
    delay(1000);  // Tunggu sebentar
    ESP.restart();  // Restart ESP32 untuk mulai konfigurasi ulang
  }

  // Mengecek koneksi WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus! Membuka portal lagi...");
    reconnectWiFi();
  }

  // Membaca jarak sensor dan menghitung tinggi air
  int jarak = bacaUltrasonik();
  int tinggiAir = kedalamanSungai - jarak;
  if (tinggiAir < 0) tinggiAir = 0;

  Serial.print("Jarak sensor ke air: ");
  Serial.print(jarak);
  Serial.println(" cm");

  Serial.print("Tinggi air: ");
  Serial.print(tinggiAir);
  Serial.println(" cm");

  // Kirim data ke Firebase (selalu)
  if (WiFi.status() == WL_CONNECTED) {
    sendToFirebase(tinggiAir);
  } else {
    Serial.println("WiFi tidak terhubung, data tidak dikirim.");
  }

  // Kirim data ke Telegram hanya jika melebihi threshold
  if (tinggiAir > thresholdTinggiAir) {
    String message = "Tinggi air sensor 1 saat ini: " + String(tinggiAir) + " cm";
    if (bot.sendMessage(chat_id, message, "")) {
      Serial.println("Pesan terkirim ke Telegram!");
    } else {
      Serial.println("Gagal mengirim pesan ke Telegram.");
    }
  } else {
    Serial.println("Tinggi air tidak mencapai threshold. Tidak mengirim pesan ke Telegram.");
  }

  delay(15000);  // Delay 15 detik sebelum membaca sensor lagi
}

int bacaUltrasonik() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH, 30000);
  int jarak = durasi * 0.034 / 2;

  if (durasi == 0) jarak = kedalamanSungai;
  return jarak;
}

void sendToFirebase(int value) {
  String sensorID = "Sensor2";
  String path = "/" + sensorID + ".json?auth=" + FIREBASE_AUTH;
  String json = "{\"value\": " + String(value) + "}";

  WiFiClientSecure client;
  client.setInsecure();  // Abaikan SSL (bisa ganti ke aman jika perlu)

  HttpClient http(client, FIREBASE_HOST, SSL_PORT);

  Serial.println("Mengirim data ke Firebase...");

  http.patch(path, "application/json", json);

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  if (statusCode == 200) {
    Serial.println("Data berhasil dikirim.");
  } else {
    Serial.println("Gagal kirim data, mencoba ulang...");
    delay(2000);
    http.patch(path, "application/json", json);
    Serial.print("Status ulang: ");
    Serial.println(http.responseStatusCode());
    Serial.print("Response ulang: ");
    Serial.println(http.responseBody());
  }

  http.stop();
}

void reconnectWiFi() {
  Serial.println("WiFi terputus, memulai portal konfigurasi...");
  wifiManager.startConfigPortal("ESP32-Access-Point-1", "12345678");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi terhubung kembali!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi tetap gagal.");
  }
}