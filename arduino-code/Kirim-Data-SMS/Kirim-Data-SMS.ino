#include <HardwareSerial.h>
#include <stdlib.h>

#define MODEM_RX 22  // ESP32 RX (ke TX SIM800L)
#define MODEM_TX 23  // ESP32 TX (ke RX SIM800L)

HardwareSerial sim800(2); // UART2

unsigned long lastSend = 0;
const unsigned long sendInterval = 10000; // 10 detik

void sendToFirebase(int value) {
  Serial.println("Mengirim data ke Firebase...");

  // HTTP init
  sim800.println("AT+HTTPTERM");
  delay(100);
  sim800.println("AT+HTTPINIT");
  delay(500);

  // Set URL Firebase (gunakan PUT untuk mengganti value)
  sim800.println("AT+HTTPPARA=\"URL\",\"https://ketinggian-air-551c0-default-rtdb.firebaseio.com/Sensor1.json\"");
  delay(1000);

  // Set Content Type
  sim800.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(500);

  // Data JSON
  String jsonData = "{\"value\": " + String(value) + "}";
  sim800.print("AT+HTTPDATA=");
  sim800.print(jsonData.length());
  sim800.println(",10000");
  delay(500);

  // Kirim data JSON
  sim800.print(jsonData);
  delay(1000);

  // Kirim POST/PUT (gunakan PUT agar menggantikan)
  sim800.println("AT+HTTPACTION=1"); // 1=POST, 0=GET, 2=PUT
  delay(5000);

  // Baca respons
  while (sim800.available()) {
    Serial.write(sim800.read());
  }

  // Selesai
  sim800.println("AT+HTTPTERM");
  delay(500);
}

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);

  Serial.println("Menginisialisasi SIM800L...");
  delay(2000);
  
  // Cek koneksi
  sim800.println("AT");
  delay(1000);
}

void loop() {
  // Tampilkan respon dari SIM800L
  while (sim800.available()) {
    Serial.write(sim800.read());
  }

  // Kirim AT manual
  while (Serial.available()) {
    sim800.write(Serial.read());
  }

  // Kirim data setiap 10 detik
  if (millis() - lastSend > sendInterval) {
    int value = random(100, 151); // 100-150
    Serial.println("Random Value: " + String(value));
    sendToFirebase(value);
    lastSend = millis();
  }
}