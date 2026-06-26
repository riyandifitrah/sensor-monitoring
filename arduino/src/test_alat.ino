#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHT_PIN   2
#define DHT_TYPE  DHT22

#define LED_HIJAU   5
#define LED_KUNING  6
#define LED_MERAH   7

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // ganti 0x3F jika 0x27 tidak tampil

void setup() {
    Serial.begin(9600);

    pinMode(LED_HIJAU,  OUTPUT);
    pinMode(LED_KUNING, OUTPUT);
    pinMode(LED_MERAH,  OUTPUT);

    // test semua LED sebentar saat startup
    digitalWrite(LED_HIJAU,  HIGH);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_MERAH,  HIGH);
    delay(800);
    digitalWrite(LED_HIJAU,  LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH,  LOW);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Test Alat...");
    delay(1000);
    lcd.clear();

    dht.begin();
    pinMode(DHT_PIN, INPUT_PULLUP);  // internal pull-up ~20-50kΩ, lemah untuk DHT22
    Serial.println("=== Test Alat Dimulai ===");
    delay(2000);
}

void matikanSemua() {
    digitalWrite(LED_HIJAU,  LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH,  LOW);
}

void loop() {
    float suhu       = dht.readTemperature();
    float kelembaban = dht.readHumidity();

    if (isnan(suhu) || isnan(kelembaban)) {
        Serial.println("ERROR: Gagal baca sensor! Cek wiring DHT22.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor Error!");
        lcd.setCursor(0, 1);
        lcd.print("Cek kabel DHT22");
        matikanSemua();
        digitalWrite(LED_MERAH, HIGH);
        delay(2000);
        return;
    }

    // Serial output
    Serial.print("Suhu: ");
    Serial.print(suhu, 1);
    Serial.print(" C  |  Kelembaban: ");
    Serial.print(kelembaban, 1);
    Serial.print(" %  |  Status: ");

    // tampil di LCD baris 1: suhu & kelembaban
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(suhu, 1);
    lcd.print("C  H:");
    lcd.print(kelembaban, 1);
    lcd.print("%");

    // fuzzy logic sederhana → status + LED
    matikanSemua();
    if (suhu > 35 || suhu < 15 || kelembaban > 80) {
        Serial.println("BAHAYA");
        lcd.setCursor(0, 1);
        lcd.print("Status: BAHAYA  ");
        digitalWrite(LED_MERAH, HIGH);
    } else if (suhu > 28 || kelembaban > 60) {
        Serial.println("WASPADA");
        lcd.setCursor(0, 1);
        lcd.print("Status: WASPADA ");
        digitalWrite(LED_KUNING, HIGH);
    } else {
        Serial.println("NYAMAN");
        lcd.setCursor(0, 1);
        lcd.print("Status: NYAMAN  ");
        digitalWrite(LED_HIJAU, HIGH);
    }

    delay(3000);
}
