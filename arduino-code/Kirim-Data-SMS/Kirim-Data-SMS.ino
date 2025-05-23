#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial
#define SerialAT Serial2
#define TINY_GSM_DEBUG SerialMon

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <FirebaseClient.h>  // https://github.com/mobizt/FirebaseClient

// Firebase Configuration
#define API_KEY "AIzaSyA0c2F8uR1t4PVsQNoTyd27_DfuNnwoSsk"
#define USER_EMAIL "test@gmail.com"
#define USER_PASSWORD "123456"
#define DATABASE_URL "https://ketinggian-air-551c0-default-rtdb.firebaseio.com/"

// GPRS Configuration
const char apn[] = "internet";   // Ganti jika APN kartu SIM berbeda
const char gprsUser[] = "";
const char gprsPass[] = "";
#define GSM_PIN ""  // PIN SIM Card jika ada

// Pin TX/RX untuk SIM800L (pastikan sesuai dengan wiring kamu)
#define MODEM_TX 0   // TX ke RX SIM800L
#define MODEM_RX 4   // RX ke TX SIM800L

// Inisialisasi objek modem dan client
TinyGsm modem(SerialAT);
TinyGsmClient gsm_client(modem, 0);

// Firebase client dan konfigurasi
ESP_SSLClient ssl_client;
GSMNetwork gsm_network(&modem, GSM_PIN, apn, gprsUser, gprsPass);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(gsm_network));
RealtimeDatabase Database;

unsigned long lastSend = 0;

// Callback untuk respon Firebase
void asyncCB(AsyncResult &aResult) {
  if (aResult.isError()) {
    Serial.printf("Error: %s, Code: %d\n", aResult.error().message().c_str(), aResult.error().code());
  } else {
    Serial.println("Data terkirim!");
  }
}

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialAT.begin(9600, SERIAL_8N1, MODEM_TX, MODEM_RX);  // Sesuaikan baudrate SIM800L

  SerialMon.println("Memulai modem...");
  modem.restart();

  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }

  SerialMon.print("Menunggu jaringan...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" Gagal koneksi jaringan");
    while (true);
  }
  SerialMon.println(" Jaringan ditemukan");

  SerialMon.print("Menghubungkan ke APN: ");
  SerialMon.println(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" Gagal koneksi GPRS");
    while (true);
  }
  SerialMon.println("GPRS terhubung");

  // Konfigurasi SSL dan Firebase
  ssl_client.setInsecure();
  ssl_client.setBufferSizes(2048, 1024);
  ssl_client.setClient(&gsm_client);

  SerialMon.println("Inisialisasi Firebase...");
  initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

void loop() {
  app.loop();
  Database.loop();

  if (millis() - lastSend > 5000 && app.ready()) {
    lastSend = millis();
    
    int dummyValue = random(0, 100);  // Data acak antara 0 dan 99
    Serial.print("Mengirim data ke /Sensor1: ");
    Serial.println(dummyValue);

    // Kirim data ke path /Sensor1
    Database.set<int>(aClient, "/Sensor1", dummyValue, asyncCB, "setDummy");
  }
}
