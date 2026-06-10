-- Sensor Monitoring System — Database Schema
-- Jalankan di phpMyAdmin atau MySQL CLI sebelum menggunakan aplikasi

CREATE DATABASE IF NOT EXISTS sensor_monitoring
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE sensor_monitoring;

CREATE TABLE IF NOT EXISTS sensor_data (
    id          INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    suhu        DECIMAL(5,2)   NOT NULL COMMENT 'Suhu dalam Celsius',
    kelembaban  DECIMAL(5,2)   NOT NULL COMMENT 'Kelembaban dalam persen',
    nilai_fuzzy DECIMAL(5,4)   NOT NULL DEFAULT 0.0000 COMMENT 'Output fuzzy (0.0–1.0)',
    status      VARCHAR(20)    NOT NULL COMMENT 'Nyaman | Waspada | Bahaya',
    waktu       DATETIME       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_waktu (waktu),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Data dummy untuk testing
INSERT INTO sensor_data (suhu, kelembaban, nilai_fuzzy, status, waktu) VALUES
(24.5, 55.0, 0.8500, 'Nyaman',   NOW() - INTERVAL 5  MINUTE),
(26.0, 58.0, 0.8200, 'Nyaman',   NOW() - INTERVAL 10 MINUTE),
(30.5, 65.0, 0.4500, 'Waspada',  NOW() - INTERVAL 15 MINUTE),
(36.0, 72.0, 0.1200, 'Bahaya',   NOW() - INTERVAL 20 MINUTE),
(22.0, 48.0, 0.9000, 'Nyaman',   NOW() - INTERVAL 25 MINUTE),
(25.0, 52.0, 0.8700, 'Nyaman',   NOW() - INTERVAL 30 MINUTE);
