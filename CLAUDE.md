# Sensor Monitoring System — CLAUDE.md

## Project Overview
Web monitoring suhu dan kelembaban berbasis Arduino + Ethernet Shield.
Data sensor dikirim dari Arduino ke server PHP via HTTP GET/POST, disimpan di MySQL, dan ditampilkan secara real-time di browser.

## Tech Stack
- **Backend**: PHP native (no framework)
- **Database**: MySQL via XAMPP
- **Frontend**: HTML, CSS, JavaScript + Bootstrap 5
- **Hardware**: Arduino + DHT sensor + Ethernet Shield (W5100/W5500)
- **Server**: XAMPP (Apache + MySQL), berjalan di localhost

## Folder Structure
```
/var/www/html/php/
├── CLAUDE.md
├── index.php               # redirect ke monitoring
├── monitoring.php          # halaman monitoring real-time
├── riwayat.php             # halaman riwayat / history
├── api/
│   ├── get_latest.php      # GET: ambil data sensor terbaru
│   ├── get_history.php     # GET: ambil riwayat dengan paginasi + search
│   ├── delete_record.php   # POST: hapus record berdasarkan id
│   └── receive_data.php    # GET/POST: endpoint untuk Arduino kirim data
├── config/
│   └── database.php        # koneksi PDO ke MySQL
├── assets/
│   ├── css/style.css       # custom styling
│   └── js/app.js           # polling real-time & interaksi UI
└── sql/
    └── schema.sql          # DDL tabel sensor_data
```

## Database
- **DB name**: `sensor_monitoring`
- **Table**: `sensor_data` (id, suhu, kelembaban, nilai_fuzzy, status, waktu)
- Koneksi via PDO dengan prepared statements (aman dari SQL injection)

## Fuzzy Logic (Mamdani sederhana)
Status ditentukan berdasarkan kombinasi suhu dan kelembaban:
| Status     | Kondisi                                      | LED    |
|------------|----------------------------------------------|--------|
| Nyaman     | suhu 18–28°C AND kelembaban 40–60%           | Hijau  |
| Waspada    | suhu 28–35°C OR kelembaban 60–80%            | Kuning |
| Bahaya     | suhu >35°C OR suhu <15°C OR kelembaban >80%  | Merah  |

Nilai fuzzy adalah output crisp (0.0–1.0) dari proses defuzzifikasi.

## Arduino Integration
- Arduino mengirim data via HTTP GET ke `api/receive_data.php`
- Format: `GET /php/api/receive_data.php?suhu=25.5&kelembaban=55.2`
- Tidak perlu autentikasi untuk kemudahan integrasi Arduino
- Ethernet Shield menggunakan library `Ethernet.h` dan `EthernetClient`

## UI Pages
### Monitoring (`monitoring.php`)
- Status label + nilai fuzzy di header
- LED indicator: Merah / Kuning / Hijau (aktif sesuai status)
- Dua card besar: SUHU dan KELEMBABAN dengan nilai real-time
- Timestamp data terakhir
- Auto-refresh via JavaScript polling setiap 3 detik

### Riwayat (`riwayat.php`)
- Tabel: No, Suhu, Kelembaban, Status, Waktu, Aksi (Hapus)
- Pagination 6 data per halaman
- Fitur pencarian berdasarkan status atau rentang waktu
- Konfirmasi sebelum hapus

## Development Notes
- Semua response API dalam format JSON
- CORS tidak diperlukan (same-origin)
- Gunakan PDO untuk semua query database
- Tidak ada framework PHP — semua vanilla PHP
- Bootstrap 5 di-load via CDN
