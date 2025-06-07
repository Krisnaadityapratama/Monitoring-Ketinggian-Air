#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Aurell_Arine";
const char* password = "aurellarine";

String botToken = "7323920625:AAHieSajecgla0UCAqQ9GrT-shyIDypWBr8";
String chatID = "6456563480";
String lokasi = "Jl Kampung Purbonegaran"; // Lokasi sensor 1

#define PIN_TRIG 12
#define PIN_ECHO 14

float ketinggianAir;
int statusSiaga = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi tersambung!");
}

void loop() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long durasi = pulseIn(PIN_ECHO, HIGH);
  ketinggianAir = (durasi * 0.0343) / 2;

  Serial.print("Tinggi air: ");
  Serial.print(ketinggianAir);
  Serial.println(" cm");

  if (ketinggianAir >= 100 && ketinggianAir < 140 && statusSiaga != 1) {
    kirimPeringatan("SIAGA 3 di " + lokasi + "\nTinggi air: " + String(ketinggianAir, 2) + " cm.");
    statusSiaga = 1;
  } else if (ketinggianAir >= 140 && ketinggianAir < 170 && statusSiaga != 2) {
    kirimPeringatan("SIAGA 2 di " + lokasi + "\nTinggi air: " + String(ketinggianAir, 2) + " cm.");
    statusSiaga = 2;
  } else if (ketinggianAir >= 170 && statusSiaga != 3) {
    kirimPeringatan("SIAGA 1 di " + lokasi + "\nTinggi air: " + String(ketinggianAir, 2) + " cm. Bahaya banjir!");
    statusSiaga = 3;
  } else if (ketinggianAir < 100 && statusSiaga != 0) {
    statusSiaga = 0;
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

String urlEncode(String str) {
  String encoded = "";
  char c;
  char bufHex[4];
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      sprintf(bufHex, "%%%02X", c);
      encoded += bufHex;
    }
  }
  return encoded;
}
