<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Monitoring Sensor</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css">
    <link rel="stylesheet" href="assets/css/style.css">
</head>
<body>
<div id="monitoring-page" class="app-wrapper">

    <!-- Sidebar -->
    <aside class="sidebar">
        <div class="sidebar-brand">
            <div class="brand-title">Sensor Monitor</div>
            <div class="brand-sub">Arduino IoT Dashboard</div>
        </div>
        <nav class="sidebar-nav">
            <a href="monitoring.php" class="nav-btn active">
                <span class="nav-icon">&#9685;</span>
                <span>Monitoring</span>
            </a>
            <a href="riwayat.php" class="nav-btn">
                <span class="nav-icon">&#9776;</span>
                <span>Riwayat</span>
            </a>
        </nav>
        <div class="sidebar-footer">v1.0 &mdash; DHT22 + W5100</div>
    </aside>

    <!-- Main Content -->
    <main class="main-content">

        <!-- Page Header -->
        <div class="page-header">
            <div>
                <h1 class="page-title">Monitoring Real-Time</h1>
                <div class="page-subtitle">Data diperbarui otomatis setiap 3 detik</div>
            </div>
            <span id="conn-status" class="conn-badge conn-loading">Memuat</span>
        </div>

        <!-- Header Bar: Status + Fuzzy + LED -->
        <div class="header-bar">
            <div class="info-pill">
                <label>Status:</label>
                <span id="val-status" class="status-nyaman">—</span>
            </div>
            <div class="info-pill">
                <label>Nilai <em>fuzzy</em>:</label>
                <span id="val-fuzzy">—</span>
            </div>

            <!-- LED Indicators -->
            <div class="led-group">
                <div class="led-item">
                    <div class="led-circle" id="led-merah"></div>
                    <span>Merah</span>
                </div>
                <div class="led-item">
                    <div class="led-circle" id="led-kuning"></div>
                    <span>Kuning</span>
                </div>
                <div class="led-item">
                    <div class="led-circle" id="led-hijau"></div>
                    <span>Hijau</span>
                </div>
            </div>
        </div>

        <!-- Sensor Cards -->
        <div class="sensor-cards">
            <div class="sensor-card card-suhu" id="card-suhu">
                <div class="sensor-icon">&#127777;</div>
                <div class="sensor-label">Suhu</div>
                <div class="sensor-value"><span id="val-suhu">--</span></div>
                <div class="sensor-unit">Celcius (&deg;C)</div>
            </div>

            <div class="sensor-card card-kelembaban" id="card-kelembaban">
                <div class="sensor-icon">&#128167;</div>
                <div class="sensor-label">Kelembaban</div>
                <div class="sensor-value"><span id="val-kelembaban">--</span></div>
                <div class="sensor-unit">Persen (%)</div>
            </div>
        </div>

        <!-- Timestamp -->
        <div class="timestamp-bar" id="val-timestamp">
            Menghubungkan ke sensor...
        </div>

    </main>
</div>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
<script src="assets/js/app.js"></script>
</body>
</html>
