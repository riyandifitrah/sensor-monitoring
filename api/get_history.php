<?php
require_once __DIR__ . '/../config/database.php';

header('Content-Type: application/json');

$page    = max(1, (int)($_GET['page']    ?? 1));
$limit   = 6;
$offset  = ($page - 1) * $limit;
$search  = trim($_GET['search'] ?? '');

try {
    $db = getDB();

    $where  = '';
    $params = [];

    if ($search !== '') {
        $where    = 'WHERE status LIKE ? OR DATE_FORMAT(waktu, "%Y-%m-%d %H:%i:%s") LIKE ?';
        $like     = '%' . $search . '%';
        $params[] = $like;
        $params[] = $like;
    }

    // Total rows
    $countStmt = $db->prepare("SELECT COUNT(*) FROM sensor_data $where");
    $countStmt->execute($params);
    $total = (int)$countStmt->fetchColumn();
    $totalPages = (int)ceil($total / $limit);

    // Data
    $dataStmt = $db->prepare(
        "SELECT id, suhu, kelembaban, nilai_fuzzy, status, waktu
         FROM sensor_data $where
         ORDER BY waktu DESC
         LIMIT $limit OFFSET $offset"
    );
    $dataStmt->execute($params);
    $rows = $dataStmt->fetchAll();

    echo json_encode([
        'success'     => true,
        'data'        => $rows,
        'page'        => $page,
        'total_pages' => $totalPages,
        'total_rows'  => $total,
    ]);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
