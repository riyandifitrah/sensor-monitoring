<!DOCTYPE html>
<html lang="id">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Riwayat Sensor</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css">
    <link rel="stylesheet" href="assets/css/style.css">
</head>

<body>
    <div id="riwayat-page" class="app-wrapper">

        <!-- Sidebar -->
        <aside class="sidebar">
            <div class="sidebar-brand">
                <div class="brand-title">Sensor Monitor</div>
                <div class="brand-sub">Arduino IoT Dashboard</div>
            </div>
            <nav class="sidebar-nav">
                <a href="monitoring.php" class="nav-btn">
                    <span class="nav-icon">&#9685;</span>
                    <span>Monitoring</span>
                </a>
                <a href="riwayat.php" class="nav-btn active">
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
                    <h1 class="page-title">Riwayat Data Sensor</h1>
                    <div class="page-subtitle">Histori pembacaan suhu dan kelembaban</div>
                </div>
                <div class="search-box">
                    <input type="text" id="search-input" placeholder="Cari status atau waktu...">
                    <button id="search-btn" class="btn-search">Cari</button>
                    <button id="reset-btn" class="btn-reset-filter">Reset</button>
                </div>
            </div>

            <!-- Table Card -->
            <div class="table-card">
                <table>
                    <thead>
                        <tr>
                            <th>No</th>
                            <th>Suhu</th>
                            <th>Kelembaban</th>
                            <th>Status</th>
                            <th>Waktu</th>
                            <th>Aksi</th>
                        </tr>
                    </thead>
                    <tbody id="tbl-body">
                        <tr>
                            <td colspan="6" class="empty-state">Memuat data...</td>
                        </tr>
                    </tbody>
                </table>

                <!-- Pagination -->
                <div class="pagination-bar">
                    <div class="page-info" id="page-info">Nomor Halaman 1 dari 1</div>
                    <div class="page-buttons" id="page-buttons"></div>
                </div>
            </div>

        </main>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
    <script src="assets/js/app.js"></script>
</body>

</html>