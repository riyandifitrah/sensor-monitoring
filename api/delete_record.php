<?php
require_once __DIR__ . '/../config/database.php';

header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    exit;
}

$id = isset($_POST['id']) ? (int)$_POST['id'] : 0;

if ($id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'ID tidak valid']);
    exit;
}

try {
    $db   = getDB();
    $stmt = $db->prepare('DELETE FROM sensor_data WHERE id = ?');
    $stmt->execute([$id]);

    if ($stmt->rowCount() === 0) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Data tidak ditemukan']);
        exit;
    }

    echo json_encode(['success' => true, 'message' => 'Data berhasil dihapus']);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
