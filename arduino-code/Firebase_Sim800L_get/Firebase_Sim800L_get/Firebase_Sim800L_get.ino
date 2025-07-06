#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>

// Konfigurasi SIM800
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

// Konfigurasi Ultrasonik
const int trigPin = 5;
const int echoPin = 18;

// Ubah nilai ini sesuai kedalaman sungai aktual dalam cm
int kedalamanSungai = 200; // contoh: 200 cm = 2 meter

void setup() {
  Serial.begin(115200);
  delay(3000);
  sim800.begin(9600);
  delay(3000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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
  int jarak = bacaUltrasonik();
  int tinggiAir = kedalamanSungai - jarak;

  // Validasi tinggi air tidak negatif
  if (tinggiAir < 0) tinggiAir = 0;

  // Tampilkan data
  Serial.print("Jarak sensor ke permukaan air: ");
  Serial.print(jarak);
  Serial.println(" cm");

  Serial.print("Tinggi air: ");
  Serial.print(tinggiAir);
  Serial.println(" cm");

  // Kirim ke Firebase
  sendToFirebase(tinggiAir);

  delay(15000); // jeda 15 detik
}

int bacaUltrasonik() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH);
  int jarak = durasi * 0.034 / 2; // konversi ke cm

  return jarak;
}

void sendToFirebase(int value) {
  String sensorID = "Sensor1"; // ubah sesuai device
  String path = "/" + sensorID + ".json?auth=" + FIREBASE_AUTH;
  String json = "{\"value\": " + String(value) + "}";

  if (http.connect(FIREBASE_HOST, SSL_PORT)) {
    http.setHttpResponseTimeout(20000);
    http.patch(path, "application/json", json);

    int statusCode = http.responseStatusCode();

    Serial.print("Status code: ");
    Serial.println(statusCode);

    if (statusCode == 200 || statusCode == 0 || statusCode == -3) {
      Serial.println("Status: BERHASIL dikirim");
    } else {
      Serial.println("Status: GAGAL dikirim");
    }

    http.stop();
  } else {
    Serial.println("Koneksi HTTP gagal");
  }
}
