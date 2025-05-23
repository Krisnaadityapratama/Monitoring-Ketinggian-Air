#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "test";
const char* password = "12345678";

String botToken = "7323920625:AAHieSajecgla0UCAqQ9GrT-shyIDypWBr8";
String chatID = "6456563480";

// Sensor Ultrasonik
#define PIN_TRIG 12
#define PIN_ECHO 14

float ketinggianAir;
bool sudahKirimNotifikasi = false;
bool sudahKirimUpdate = false; // ⬅️ Tambahan baru

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
  // Baca sensor ultrasonik
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

  delay(60000); // baca tiap 1 menit
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
