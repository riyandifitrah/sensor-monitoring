#include <DHT.h>

#define DHT_PIN   A0   // kalau kabel DATA memang di A0
#define DHT_TYPE  DHT11   // ganti DHT22 jika pakai DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
    Serial.begin(9600);
    dht.begin();
    Serial.println("DHT Test Start...");
    delay(1000);
}

void loop() {
    float suhu       = dht.readTemperature();
    float kelembaban = dht.readHumidity();

    if (isnan(suhu) || isnan(kelembaban)) {
        Serial.println("ERROR: Gagal baca sensor! Cek wiring.");
    } else {
        Serial.print("Suhu: ");    Serial.print(suhu);    Serial.print(" C  |  ");
        Serial.print("Kelembaban: "); Serial.print(kelembaban); Serial.println(" %");
    }

    delay(2000);
}
