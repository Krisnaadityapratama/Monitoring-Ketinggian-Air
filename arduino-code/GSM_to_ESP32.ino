#include <HardwareSerial.h>
#include <Arduino.h>

#define MODEM_RX 27
#define MODEM_TX 26
HardwareSerial sim800(1);

// Firebase info
const String FIREBASE_HOST = "ketinggian-air-sungai-belik-default-rtdb.firebaseio.com";
const String FIREBASE_PATH = "/Sensor1.json";  // ".json" wajib!
const String FIREBASE_TOKEN = "";  // Kosongin kalau pakai rules terbuka

// Sensor
#define PIN_TRIG 12
#define PIN_ECHO 14

long durasi;
float cm;

void sendAT(String cmd, int delayTime = 1000) {
  sim800.println(cmd);
  delay(delayTime);
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

String sendATread(String cmd, int timeout = 2000) {
  String response = "";
  sim800.println(cmd);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (sim800.available()) {
      response += char(sim800.read());
    }
  }
  return response;
}

bool setupGPRS() {
  sendAT("AT");
  sendAT("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  sendAT("AT+SAPBR=3,1,\"APN\",\"internet\"");
  sendAT("AT+SAPBR=1,1", 3000);
  String ip = sendATread("AT+SAPBR=2,1", 3000);
  return ip.indexOf("0.0.0.0") == -1;
}

bool sendDataToFirebase(float ketinggian) {
  String json = "{\"tinggi\":" + String(ketinggian) + "}";
  String httpCmd = "AT+HTTPINIT";
  sendAT(httpCmd);

  sendAT("AT+HTTPPARA=\"CID\",1");
  sendAT("AT+HTTPPARA=\"URL\",\"http://" + FIREBASE_HOST + FIREBASE_PATH + "\"");

  sendAT("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  sendAT("AT+HTTPDATA=" + String(json.length()) + ",10000");
  delay(100); // Tunggu ready
  sim800.print(json); // Kirim data JSON
  delay(2000);

  sendAT("AT+HTTPACTION=1", 6000); // POST
  String response = sendATread("AT+HTTPREAD", 3000);

  sendAT("AT+HTTPTERM");

  Serial.println("Firebase response: " + response);
  return response.indexOf("OK") != -1;
}

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(2000);
  Serial.println("Setup GPRS...");
  if (setupGPRS()) {
    Serial.println("GPRS Connected");
  } else {
    Serial.println("Gagal konek GPRS");
  }

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

void loop() {
  // Baca sensor
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  durasi = pulseIn(PIN_ECHO, HIGH);
  cm = durasi * 0.0343 / 2;

  Serial.print("Ketinggian: ");
  Serial.print(cm);
  Serial.println(" cm");

  // Kirim ke Firebase
  if (sendDataToFirebase(cm)) {
    Serial.println("✅ Data terkirim ke Firebase!");
  } else {
    Serial.println("❌ Gagal kirim ke Firebase!");
  }

  delay(10000); // Tiap 10 detik
}
