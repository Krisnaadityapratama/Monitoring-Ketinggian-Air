#define TRIG_PIN 5   // Pin Trig sensor
#define ECHO_PIN 18  // Pin Echo sensor

long duration;
float distance;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("=== UJI KALIBRASI SENSOR ULTRASONIK ===");
  Serial.println("=========================================");
}

void loop() {
  // Kirim pulsa trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca waktu pantulan
  duration = pulseIn(ECHO_PIN, HIGH);

  // Konversi ke cm (kecepatan suara 343 m/s)
  distance = duration * 0.0343 / 2; // dalam cm

  // Tampilkan hasil
  Serial.print("Jarak terukur: ");
  Serial.print(distance, 2); // dua angka di belakang koma
  Serial.println(" cm");

  delay(5000); // jeda 1 detik
}
