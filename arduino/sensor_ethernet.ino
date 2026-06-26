/*
 * Sensor Monitoring System — Arduino Uno R3
 * Hardware : Arduino Uno R3 + Modul W5500 + DHT11 + LCD I2C 16x2
 *            + 3x LED (Hijau, Kuning, Merah)
 *
 * Wiring Modul W5500 → Arduino Uno R3:
 *   VCC  → 3.3V  (cek label modul — ada yang support 5V jika ada LDO onboard)
 *   GND  → GND
 *   MISO → D12
 *   MOSI → D11
 *   SCLK → D13
 *   CS   → D10
 *   RST  → (opsional) D9 atau biarkan kosong
 *   INT  → biarkan kosong
 *
 * Wiring lainnya:
 *   DHT11      → VCC=5V, GND=GND, DATA=D2
 *   LCD I2C    → VCC=5V, GND=GND, SDA=A4, SCL=A5
 *   LED Hijau  → D5 → resistor 220Ω → GND   (Nyaman)
 *   LED Kuning → D6 → resistor 220Ω → GND   (Waspada)
 *   LED Merah  → D7 → resistor 220Ω → GND   (Bahaya)
 *
 * Alur data:
 *   1. Arduino baca DHT11 → suhu + kelembaban (raw)
 *   2. Kirim ke PHP via HTTP GET
 *   3. PHP hitung Fuzzy Logic → balas JSON { status, nilai_fuzzy, ... }
 *   4. Arduino parse response → tampil di LCD + nyalakan LED yang sesuai
 *
 * Libraries (Arduino IDE Library Manager):
 *   - "Ethernet" by Arduino (built-in, v2 sudah support W5500)
 *   - "DHT sensor library" by Adafruit  (+ Adafruit Unified Sensor)
 *   - "LiquidCrystal I2C" by Frank de Brabander
 *   - SPI, Wire  (built-in)
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ============================================================
//  KONFIGURASI — sesuaikan sebelum upload
// ============================================================
#define DHT_PIN    2
// #define DHT_TYPE   DHT22
#define DHT_TYPE   DHT11   // ganti dari DHT22

#define LCD_ADDR   0x27   // coba 0x3F jika LCD tidak tampil
#define LCD_COLS   16
#define LCD_ROWS   2

// Pin LED fisik
#define PIN_LED_HIJAU   5
#define PIN_LED_KUNING  6
#define PIN_LED_MERAH   7

// MAC address — unik per perangkat
byte MAC_ADDR[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP Arduino (dipakai hanya jika DHCP gagal)
IPAddress ARDUINO_IP(192, 168, 1, 177);

// IP komputer tempat Apache+PHP berjalan
// Cek: Windows → ipconfig | Linux → ip a
IPAddress SERVER_IP(192, 168, 1, 100);
const int  SERVER_PORT = 80;
const char* API_PATH   = "/php/api/receive_data.php";

const unsigned long SEND_INTERVAL = 5000;  // kirim data tiap 5 detik
const unsigned long LCD_INTERVAL  = 1000;  // refresh LCD tiap 1 detik
// ============================================================

DHT               dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
EthernetClient    client;

unsigned long lastSend = 0;
unsigned long lastLCD  = 0;

// Data sensor terakhir yang berhasil dibaca
float  lastSuhu       = NAN;
float  lastKelembaban = NAN;

// Hasil fuzzy dari server
char   lastStatus[12] = "---";      // "Nyaman" / "Waspada" / "Bahaya"
float  lastFuzzy      = 0.0;
bool   hasServerData  = false;      // true setelah terima response valid pertama kali

// ============================================================
//  SETUP
// ============================================================
void setup() {
    Serial.begin(9600);

    // LED
    pinMode(PIN_LED_HIJAU,  OUTPUT);
    pinMode(PIN_LED_KUNING, OUTPUT);
    pinMode(PIN_LED_MERAH,  OUTPUT);
    setLED("---");  // semua LED mati saat start

    // LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcdShow("Sensor Monitor", "Inisialisasi...");

    // DHT22 butuh ≥2 detik warm-up
    dht.begin();
    delay(2000);

    // Ethernet — coba DHCP, fallback ke statis
    lcdShow("Ethernet...", "Menghubungkan");
    Serial.println(F("Init Ethernet..."));

    if (Ethernet.begin(MAC_ADDR) == 0) {
        Serial.println(F("DHCP gagal -> IP statis"));
        Ethernet.begin(MAC_ADDR, ARDUINO_IP);
    }
    delay(1500);

    IPAddress ip = Ethernet.localIP();
    char ipBuf[16];
    snprintf(ipBuf, sizeof(ipBuf), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    Serial.print(F("IP Arduino: ")); Serial.println(ipBuf);
    lcdShow("IP:", ipBuf);
    delay(2000);
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
    Ethernet.maintain();  // jaga DHCP lease tetap valid

    unsigned long now = millis();

    if (now - lastSend >= SEND_INTERVAL) {
        lastSend = now;
        readAndSend();
    }

    if (now - lastLCD >= LCD_INTERVAL) {
        lastLCD = now;
        updateLCD();
    }
}

// ============================================================
//  BACA SENSOR & KIRIM KE SERVER
// ============================================================
void readAndSend() {
    float suhu       = dht.readTemperature();
    float kelembaban = dht.readHumidity();

    if (isnan(suhu) || isnan(kelembaban)) {
        Serial.println(F("[ERROR] Gagal baca DHT22"));
        lcdShow("DHT22 ERROR!", "Cek kabel DATA");
        setLED("---");
        return;
    }

    lastSuhu       = suhu;
    lastKelembaban = kelembaban;

    Serial.print(F("Suhu: "));    Serial.print(suhu, 1);
    Serial.print(F("C | Kel: ")); Serial.print(kelembaban, 1);
    Serial.println(F("%"));

    sendHTTP(suhu, kelembaban);
}

// ============================================================
//  HTTP GET → SERVER, PARSE JSON RESPONSE
//
//  Response PHP (contoh):
//  {"success":true,"id":12,"suhu":25.5,"kelembaban":60.2,
//   "nilai_fuzzy":0.8500,"status":"Nyaman"}
// ============================================================
void sendHTTP(float suhu, float kelembaban) {
    if (!client.connect(SERVER_IP, SERVER_PORT)) {
        Serial.println(F("[ERROR] Gagal konek server"));
        lcdShow("SERVER ERROR!", "Cek kabel LAN");
        return;
    }

    char query[128];
    snprintf(query, sizeof(query), "%s?suhu=%.1f&kelembaban=%.1f",
             API_PATH, suhu, kelembaban);

    // HTTP/1.0 → Apache tidak pakai chunked encoding → lebih mudah diparsing
    client.print(F("GET "));
    client.print(query);
    client.println(F(" HTTP/1.0"));
    client.print(F("Host: "));
    client.println(SERVER_IP);
    client.println(F("Connection: close"));
    client.println();

    // Baca seluruh response ke dalam buffer string
    String body = "";
    unsigned long timeout = millis();

    while (millis() - timeout < 6000) {
        while (client.available()) {
            body += (char)client.read();
            timeout = millis();  // reset timeout selama data masih mengalir
        }
        if (!client.connected()) break;
    }
    client.stop();

    Serial.print(F("Response: ")); Serial.println(body);

    // Parse JSON sederhana (tanpa library tambahan)
    if (body.indexOf(F("\"success\":true")) < 0) {
        Serial.println(F("[WARN] Server tidak konfirmasi sukses"));
        return;
    }

    // Ekstrak "status":"..."
    int idxSt = body.indexOf(F("\"status\":\""));
    if (idxSt >= 0) {
        idxSt += 10;  // lewati `"status":"`
        int idxEnd = body.indexOf('"', idxSt);
        String st = body.substring(idxSt, idxEnd);
        st.toCharArray(lastStatus, sizeof(lastStatus));
    }

    // Ekstrak "nilai_fuzzy":...
    int idxFz = body.indexOf(F("\"nilai_fuzzy\":"));
    if (idxFz >= 0) {
        idxFz += 14;  // lewati `"nilai_fuzzy":`
        int idxEnd = body.indexOf(',', idxFz);
        if (idxEnd < 0) idxEnd = body.indexOf('}', idxFz);
        lastFuzzy = body.substring(idxFz, idxEnd).toFloat();
    }

    hasServerData = true;
    setLED(lastStatus);

    Serial.print(F("[OK] Status: ")); Serial.print(lastStatus);
    Serial.print(F(" | Fuzzy: "));   Serial.println(lastFuzzy, 4);
}

// ============================================================
//  UPDATE LCD — non-blocking, dipanggil tiap LCD_INTERVAL
//
//  Layout LCD 16x2:
//    Baris 1: "S: 25.5C Nyaman "   (suhu + status fuzzy)
//    Baris 2: "K: 60.2% F:0.850"   (kelembaban + nilai fuzzy)
// ============================================================
void updateLCD() {
    if (isnan(lastSuhu) || isnan(lastKelembaban)) {
        lcdShow("Menunggu sensor", "DHT22...");
        return;
    }

    char baris1[17], baris2[17];

    if (hasServerData) {
        // Status max 7 karakter: "Nyaman"(6) / "Waspada"(7) / "Bahaya"(6)
        snprintf(baris1, sizeof(baris1), "S:%5.1fC %-7s", lastSuhu, lastStatus);
        snprintf(baris2, sizeof(baris2), "K:%5.1f%% F:%5.3f", lastKelembaban, lastFuzzy);
    } else {
        // Server belum balas — tampilkan raw saja
        snprintf(baris1, sizeof(baris1), "Suhu  :%6.1f C ", lastSuhu);
        snprintf(baris2, sizeof(baris2), "Kel   :%6.1f %% ", lastKelembaban);
    }

    // Tulis tanpa lcd.clear() → tidak flicker
    lcd.setCursor(0, 0); lcd.print(baris1);
    lcd.setCursor(0, 1); lcd.print(baris2);
}

// ============================================================
//  KONTROL LED FISIK
//  Hanya satu LED yang menyala sesuai hasil fuzzy
// ============================================================
void setLED(const char* status) {
    digitalWrite(PIN_LED_HIJAU,  LOW);
    digitalWrite(PIN_LED_KUNING, LOW);
    digitalWrite(PIN_LED_MERAH,  LOW);

    if      (strcmp(status, "Nyaman")  == 0) digitalWrite(PIN_LED_HIJAU,  HIGH);
    else if (strcmp(status, "Waspada") == 0) digitalWrite(PIN_LED_KUNING, HIGH);
    else if (strcmp(status, "Bahaya")  == 0) digitalWrite(PIN_LED_MERAH,  HIGH);
    // "---" atau tidak dikenal → semua mati
}

// ============================================================
//  HELPER: Tampilkan pesan status di LCD (pakai clear)
//  Hanya untuk pesan startup/error — bukan data real-time
// ============================================================
void lcdShow(const char* baris1, const char* baris2) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(baris1);
    lcd.setCursor(0, 1); lcd.print(baris2);
}
