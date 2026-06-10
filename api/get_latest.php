<?php
require_once __DIR__ . '/../config/database.php';

header('Content-Type: application/json');

try {
    $db   = getDB();
    $stmt = $db->query('SELECT * FROM sensor_data ORDER BY waktu DESC LIMIT 1');
    $row  = $stmt->fetch();

    if (!$row) {
        echo json_encode(['success' => false, 'message' => 'Belum ada data']);
        exit;
    }

    echo json_encode(['success' => true, 'data' => $row]);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
