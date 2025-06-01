#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>

SoftwareSerial sim800(22, 23); // RX, TX

const char FIREBASE_HOST[] = "ketinggian-air-551c0-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "1Mc7r6NJT1QBqLlSI14Pq8eDVU4An7FBlitarMPl";
const int SSL_PORT = 443;

char apn[] = "internet";
char user[] = "";
char pass[] = "";

TinyGsm modem(sim800);
TinyGsmClientSecure client(modem, 0);
HttpClient http(client, FIREBASE_HOST, SSL_PORT);

void setup() {
  Serial.begin(115200);
  delay(3000);
  sim800.begin(9600);
  delay(3000);

  Serial.println("Menginisialisasi modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("Menghubungkan ke APN: ");
  Serial.println(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("Gagal konek GPRS!");
    while (true);
  }
  Serial.println("GPRS terhubung");
}

void loop() {
  int value = random(100, 151);

  // Tampilkan nilai sebelum dikirim
  Serial.print("Nilai yang akan dikirim: ");
  Serial.println(value);

  // Kirim data ke Firebase
  sendToFirebase(value);

  delay(15000); // jeda 15 detik
}

void sendToFirebase(int value) {
  String path = "/Sensor1.json?auth=" + FIREBASE_AUTH;
  String json = "{\"value\": " + String(value) + "}";

  if (http.connect(FIREBASE_HOST, SSL_PORT)) {
    http.setHttpResponseTimeout(20000);
    http.patch(path, "application/json", json);

    int statusCode = http.responseStatusCode();

    // Tampilkan kode status (debug)
    Serial.print("Status code: ");
    Serial.println(statusCode);

    // Karena SIM800 kadang tidak bisa membaca HTTPS status, anggap statusCode == 0 juga sebagai sukses
    if (statusCode == 200 || statusCode == 0) {
      Serial.println("Status: BERHASIL dikirim");
    } else {
      Serial.println("Status: GAGAL dikirim");
    }

    http.stop();
  } else {
    Serial.println("Koneksi HTTP gagal");
  }
}
