#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// HardwareSerial pada UART2
#define MODEM_RX 16
#define MODEM_TX 17

HardwareSerial modemSerial(2); // UART2 (Serial2)

const char FIREBASE_HOST[] = "ketinggian-air-551c0-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "1Mc7r6NJT1QBqLlSI14Pq8eDVU4An7FBlitarMPl";
const int SSL_PORT = 443;

char apn[] = "internet";
char user[] = "";
char pass[] = "";

TinyGsm modem(modemSerial);
TinyGsmClientSecure client(modem, 0);
HttpClient http(client, FIREBASE_HOST, SSL_PORT);

// Sensor Ultrasonik
const int trigPin = 5;
const int echoPin = 18;
int kedalamanSungai = 200; // cm

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Inisialisasi modem
  modemSerial.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Menginisialisasi modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("Status SIM: ");
  Serial.println(modem.getSimStatus());

  Serial.print("Sinyal GSM: ");
  Serial.println(modem.getSignalQuality()); // Cek kualitas sinyal

  Serial.print("Menghubungkan ke APN: ");
  Serial.println(apn);

  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("Gagal konek GPRS!");
    while (true); // berhenti di sini jika gagal koneksi GPRS
  }

  Serial.println("GPRS terhubung");

  String ip = modem.getLocalIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  int jarak = bacaUltrasonik();
  int tinggiAir = kedalamanSungai - jarak;
  if (tinggiAir < 0) tinggiAir = 0;

  Serial.print("Jarak sensor ke permukaan air: ");
  Serial.print(jarak);
  Serial.println(" cm");

  Serial.print("Tinggi air: ");
  Serial.print(tinggiAir);
  Serial.println(" cm");

  sendToFirebase(tinggiAir);

  delay(15000);
}

int bacaUltrasonik() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH);
  int jarak = durasi * 0.034 / 2;

  return jarak;
}

void sendToFirebase(int value) {
  String sensorID = "Sensor2"; // ubah sesuai device
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