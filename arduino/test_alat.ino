/*
 * test_alat.ino — Test hardware TANPA Ethernet
 * Cek: DHT22 + LCD I2C 16x2 + 3x LED
 *
 * Wiring:
 *   DHT22      → pin 2 (DATA), 5V, GND
 *               + resistor pull-up 10kΩ antara DATA dan 5V
 *   LCD I2C    → A4 (SDA), A5 (SCL), 5V, GND
 *   LED Hijau  → pin 5 → resistor 220Ω → GND
 *   LED Kuning → pin 6 → resistor 220Ω → GND
 *   LED Merah  → pin 7 → resistor 220Ω → GND
 *
 * Status LED (hitung lokal, tanpa server):
 *   Hijau  = suhu 18–28°C DAN kelembaban 40–60%
 *   Kuning = suhu 28–35°C ATAU kelembaban 60–80%
 *   Merah  = suhu >35°C ATAU suhu <15°C ATAU kelembaban >80%
 *
 * LCD tidak pakai clear() di loop supaya tidak berkedip.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHT_PIN    2
#define DHT_TYPE   DHT22

#define LED_HIJAU   5
#define LED_KUNING  6
#define LED_MERAH   7

#define LCD_ADDR  0x27   // coba 0x3F kalau LCD tidak tampil
#define LCD_COLS  16
#define LCD_ROWS  2

DHT               dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void matikanSemua() {
    digitalWrite(LED_HIJAU,  LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH,  LOW);
}

void setup() {
    Serial.begin(9600);

    pinMode(LED_HIJAU,  OUTPUT);
    pinMode(LED_KUNING, OUTPUT);
    pinMode(LED_MERAH,  OUTPUT);

    // Self-test: nyalakan semua LED sebentar
    digitalWrite(LED_HIJAU,  HIGH);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_MERAH,  HIGH);

    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0); lcd.print("Test Alat v1.0  ");
    lcd.setCursor(0, 1); lcd.print("LED test...     ");
    Serial.println(F("=== Test Alat Dimulai ==="));

    delay(1500);
    matikanSemua();

    dht.begin();
    lcd.setCursor(0, 0); lcd.print("DHT22 warmup... ");
    lcd.setCursor(0, 1); lcd.print("Tunggu 2 detik  ");
    delay(2000);  // DHT22 butuh waktu warmup
}

void loop() {
    float suhu       = dht.readTemperature();
    float kelembaban = dht.readHumidity();

    if (isnan(suhu) || isnan(kelembaban)) {
        Serial.println(F("[ERROR] Gagal baca DHT22 — cek kabel pin 2 & pull-up 10k"));
        lcd.setCursor(0, 0); lcd.print("DHT22 ERROR!    ");
        lcd.setCursor(0, 1); lcd.print("Cek kabel pin 2 ");
        matikanSemua();
        // Kedipkan LED merah sebagai sinyal error
        digitalWrite(LED_MERAH, HIGH); delay(300);
        digitalWrite(LED_MERAH, LOW);  delay(300);
        return;
    }

    // Tentukan status
    const char* status;
    matikanSemua();
    if (suhu > 35.0 || suhu < 15.0 || kelembaban > 80.0) {
        status = "BAHAYA ";
        digitalWrite(LED_MERAH,  HIGH);
    } else if (suhu >= 28.0 || kelembaban >= 60.0) {
        status = "WASPADA";
        digitalWrite(LED_KUNING, HIGH);
    } else {
        status = "NYAMAN ";
        digitalWrite(LED_HIJAU,  HIGH);
    }

    // LCD — tulis tanpa clear() supaya tidak flicker
    // Baris 1: "S: 27.5C  NYAMAN "   (16 char)
    // Baris 2: "K: 60.2%           "
    char baris1[17], baris2[17];
    snprintf(baris1, sizeof(baris1), "S:%5.1fC  %-7s", suhu, status);
    snprintf(baris2, sizeof(baris2), "K:%5.1f%%          ", kelembaban);

    lcd.setCursor(0, 0); lcd.print(baris1);
    lcd.setCursor(0, 1); lcd.print(baris2);

    // Serial Monitor
    Serial.print(F("Suhu: "));       Serial.print(suhu, 1);       Serial.print(F(" C  |  "));
    Serial.print(F("Kelembaban: ")); Serial.print(kelembaban, 1); Serial.print(F(" %  |  "));
    Serial.println(status);

    delay(2000);
}
