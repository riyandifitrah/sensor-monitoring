<?php
/**
 * Endpoint untuk menerima data dari Arduino via Ethernet Shield.
 * Contoh request dari Arduino:
 *   GET /php/api/receive_data.php?suhu=25.5&kelembaban=55.2
 */
require_once __DIR__ . '/../config/database.php';

header('Content-Type: application/json');

// Ambil parameter (support GET dari Arduino maupun POST dari form)
$suhu       = isset($_REQUEST['suhu'])       ? (float)$_REQUEST['suhu']       : null;
$kelembaban = isset($_REQUEST['kelembaban']) ? (float)$_REQUEST['kelembaban'] : null;

if ($suhu === null || $kelembaban === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Parameter suhu dan kelembaban wajib diisi']);
    exit;
}

// Validasi rentang sensor
if ($suhu < -40 || $suhu > 80 || $kelembaban < 0 || $kelembaban > 100) {
    http_response_code(422);
    echo json_encode(['success' => false, 'message' => 'Nilai sensor di luar rentang valid']);
    exit;
}

// --- Fuzzy Logic (Mamdani sederhana) ---

// Trapezoid membership: naik dari a→b, plateau b→c, turun c→d
// Untuk "shoulder" kiri (dingin/kering): gunakan shoulderLeft()
// Untuk "shoulder" kanan (panas/lembab): gunakan shoulderRight()
function trapezoid(float $val, float $a, float $b, float $c, float $d): float {
    if ($val <= $a || $val >= $d) return 0.0;
    if ($val >= $b && $val <= $c) return 1.0;
    if ($val < $b) return ($b > $a) ? ($val - $a) / ($b - $a) : 1.0;
    return ($d > $c) ? ($d - $val) / ($d - $c) : 1.0;
}

// Shoulder kiri: nilai ≤ c → 1.0, lalu turun c→d
function shoulderLeft(float $val, float $c, float $d): float {
    if ($val <= $c) return 1.0;
    if ($val >= $d) return 0.0;
    return ($d - $val) / ($d - $c);
}

// Shoulder kanan: naik a→b, lalu nilai ≥ b → 1.0
function shoulderRight(float $val, float $a, float $b): float {
    if ($val >= $b) return 1.0;
    if ($val <= $a) return 0.0;
    return ($val - $a) / ($b - $a);
}

// Membership suhu
// ≤18     : dingin
// 18–30   : normal penuh
// ≥30     : panas
// Makna:
// ≤18 = 1 (pasti dingin)
// 18–20 = turun bertahap
// ≥20 = 0
$suhu_dingin = shoulderLeft($suhu, 18, 20);
// Makna:
// 18–20 = naik
// 20–28 = full nyaman (μ = 1)
// 28–30 = turun
$suhu_nyaman = trapezoid($suhu, 18, 20, 28, 30);
// Makna:
// ≤28 = 0
// 28–30 = naik
// ≥30 = 1
$suhu_panas = shoulderRight($suhu,28,30);

// Membership kelembaban
$kel_kering = shoulderLeft($kelembaban, 40, 45);
// Makna:
// 45–55 = kondisi ideal (μ = 1)
// 40–45 & 55–60 = transisi
$kel_nyaman = trapezoid($kelembaban, 40, 45, 55, 60);
// Makna:
// ≤40 = kering penuh
// 40–45 = transisi
$kel_lembab = shoulderRight($kelembaban, 55, 60);

// Rule base → defuzzifikasi centroid sederhana
$r_nyaman   = min($suhu_nyaman, $kel_nyaman);
$r_waspada  = max(
    min($suhu_nyaman, $kel_lembab),
    min($suhu_panas,  $kel_nyaman),
    min($suhu_dingin, $kel_nyaman)
);
$r_bahaya   = max(
    min($suhu_panas,  $kel_lembab),
    min($suhu_panas,  $kel_kering),
    min($suhu_dingin, $kel_kering),
    min($suhu_dingin, $kel_lembab)
);

// Centroid: nyaman=0.85, waspada=0.45, bahaya=0.10
$num = ($r_nyaman * 0.85) + ($r_waspada * 0.45) + ($r_bahaya * 0.10);
$den = $r_nyaman + $r_waspada + $r_bahaya;
$nilai_fuzzy = $den > 0 ? round($num / $den, 4) : 0.0;

// Tentukan status dominan
if ($r_nyaman >= $r_waspada && $r_nyaman >= $r_bahaya) {
    $status = 'Nyaman';
} elseif ($r_waspada >= $r_bahaya) {
    $status = 'Waspada';
} else {
    $status = 'Bahaya';
}

// Simpan ke database
try {
    $db   = getDB();
    $stmt = $db->prepare(
        'INSERT INTO sensor_data (suhu, kelembaban, nilai_fuzzy, status) VALUES (?, ?, ?, ?)'
    );
    $stmt->execute([$suhu, $kelembaban, $nilai_fuzzy, $status]);
    $id = $db->lastInsertId();

    echo json_encode([
        'success'     => true,
        'id'          => (int)$id,
        'suhu'        => $suhu,
        'kelembaban'  => $kelembaban,
        'nilai_fuzzy' => $nilai_fuzzy,
        'status'      => $status,
    ]);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
}
