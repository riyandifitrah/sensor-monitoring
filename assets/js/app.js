/* ============================================================
   SENSOR MONITORING SYSTEM — Frontend JS
   ============================================================ */

/* ---- Monitoring Page ---- */
(function monitoringInit() {
    const page = document.getElementById('monitoring-page');
    if (!page) return;

    const elSuhu        = document.getElementById('val-suhu');
    const elKelembaban  = document.getElementById('val-kelembaban');
    const elStatus      = document.getElementById('val-status');
    const elFuzzy       = document.getElementById('val-fuzzy');
    const elTimestamp   = document.getElementById('val-timestamp');
    const elConn        = document.getElementById('conn-status');
    const cardSuhu      = document.getElementById('card-suhu');
    const cardKelembaban= document.getElementById('card-kelembaban');
    const ledMerah      = document.getElementById('led-merah');
    const ledKuning     = document.getElementById('led-kuning');
    const ledHijau      = document.getElementById('led-hijau');

    let prevId = null;

    function setLED(status) {
        ledMerah.classList.remove('led-on-merah');
        ledKuning.classList.remove('led-on-kuning');
        ledHijau.classList.remove('led-on-hijau');

        if (status === 'Bahaya')  ledMerah.classList.add('led-on-merah');
        else if (status === 'Waspada') ledKuning.classList.add('led-on-kuning');
        else                      ledHijau.classList.add('led-on-hijau');
    }

    function setStatusStyle(el, status) {
        el.classList.remove('status-nyaman', 'status-waspada', 'status-bahaya');
        if (status === 'Nyaman')  el.classList.add('status-nyaman');
        else if (status === 'Waspada') el.classList.add('status-waspada');
        else                      el.classList.add('status-bahaya');
    }

    function flashCard(card) {
        card.classList.remove('updated');
        // Force reflow untuk restart animasi
        void card.offsetWidth;
        card.classList.add('updated');
    }

    function fetchLatest() {
        fetch('api/get_latest.php')
            .then(r => r.json())
            .then(res => {
                elConn.textContent = 'Online';
                elConn.className = 'conn-badge conn-online';

                if (!res.success) {
                    elSuhu.textContent       = '--';
                    elKelembaban.textContent = '--';
                    elStatus.textContent     = 'Menunggu data...';
                    elFuzzy.textContent      = '-';
                    elTimestamp.textContent  = 'Belum ada data sensor';
                    return;
                }

                const d = res.data;

                // Animasi hanya jika ada data baru
                if (d.id !== prevId) {
                    flashCard(cardSuhu);
                    flashCard(cardKelembaban);
                    prevId = d.id;
                }

                elSuhu.textContent       = parseFloat(d.suhu).toFixed(1);
                elKelembaban.textContent = parseFloat(d.kelembaban).toFixed(1);
                elStatus.textContent     = d.status;
                elFuzzy.textContent      = parseFloat(d.nilai_fuzzy).toFixed(4);
                elTimestamp.textContent  = 'Update terakhir: ' + d.waktu;

                setStatusStyle(elStatus, d.status);
                setLED(d.status);
            })
            .catch(() => {
                elConn.textContent = 'Offline';
                elConn.className   = 'conn-badge conn-offline';
            });
    }

    fetchLatest();
    setInterval(fetchLatest, 3000);
})();


/* ---- Riwayat Page ---- */
(function riwayatInit() {
    const page = document.getElementById('riwayat-page');
    if (!page) return;

    const tbody     = document.getElementById('tbl-body');
    const pageInfo  = document.getElementById('page-info');
    const pageBtns  = document.getElementById('page-buttons');
    const searchInput = document.getElementById('search-input');
    const searchBtn   = document.getElementById('search-btn');
    const resetBtn = document.getElementById('reset-btn'); // <-- 1. Tambahkan selector tombol reset

    let currentPage  = 1;
    let totalPages   = 1;
    let currentSearch = '';

    function statusBadgeClass(status) {
        if (status === 'Nyaman')  return 'badge-nyaman';
        if (status === 'Waspada') return 'badge-waspada';
        return 'badge-bahaya';
    }

    function renderRows(data) {
        if (data.length === 0) {
            tbody.innerHTML = `<tr><td colspan="6" class="empty-state">Tidak ada data ditemukan</td></tr>`;
            return;
        }

        tbody.innerHTML = data.map((row, idx) => {
            const no     = (currentPage - 1) * 6 + idx + 1;
            const badge  = statusBadgeClass(row.status);
            const suhu   = parseFloat(row.suhu).toFixed(1);
            const kel    = parseFloat(row.kelembaban).toFixed(1);
            return `
            <tr>
                <td>${no}.</td>
                <td>${suhu}°C</td>
                <td>${kel}%</td>
                <td><span class="status-badge ${badge}">${row.status}</span></td>
                <td>${row.waktu}</td>
                <td>
                    <button class="btn-hapus" data-id="${row.id}">Hapus</button>
                </td>
            </tr>`;
        }).join('');

        // Attach delete handlers
        tbody.querySelectorAll('.btn-hapus').forEach(btn => {
            btn.addEventListener('click', function () {
                const id = this.dataset.id;
                if (!confirm('Hapus data ini?')) return;
                deleteRecord(id);
            });
        });
    }

    function renderPagination(page, total) {
        pageInfo.textContent = `Nomor Halaman ${page} dari ${total}`;

        let html = `<span style="font-size:0.82rem;color:#666;margin-right:4px;">Page</span>`;
        html += `<button class="page-btn" id="btn-prev" ${page <= 1 ? 'disabled' : ''}>‹</button>`;

        // Window of 3 pages around current
        const start = Math.max(1, page - 1);
        const end   = Math.min(total, start + 2);

        for (let i = start; i <= end; i++) {
            html += `<button class="page-btn ${i === page ? 'active' : ''}" data-p="${i}">${i}</button>`;
        }

        if (end < total) {
            html += `<span class="page-btn" style="border:none;background:transparent;">…</span>`;
            html += `<button class="page-btn" data-p="${total}">${total}</button>`;
        }

        html += `<button class="page-btn" id="btn-next" ${page >= total ? 'disabled' : ''}>›</button>`;
        pageBtns.innerHTML = html;

        pageBtns.querySelectorAll('.page-btn[data-p]').forEach(btn => {
            btn.addEventListener('click', function () {
                loadPage(parseInt(this.dataset.p));
            });
        });

        const prev = pageBtns.querySelector('#btn-prev');
        const next = pageBtns.querySelector('#btn-next');
        if (prev) prev.addEventListener('click', () => loadPage(page - 1));
        if (next) next.addEventListener('click', () => loadPage(page + 1));
    }

    function loadPage(p) {
        currentPage = p;
        const url   = `api/get_history.php?page=${p}&search=${encodeURIComponent(currentSearch)}`;

        fetch(url)
            .then(r => r.json())
            .then(res => {
                if (!res.success) { tbody.innerHTML = `<tr><td colspan="6" class="empty-state">${res.message}</td></tr>`; return; }
                totalPages = res.total_pages;
                renderRows(res.data);
                renderPagination(res.page, res.total_pages);
            })
            .catch(() => {
                tbody.innerHTML = `<tr><td colspan="6" class="empty-state text-danger">Gagal memuat data</td></tr>`;
            });
    }

    function deleteRecord(id) {
        const fd = new FormData();
        fd.append('id', id);
        fetch('api/delete_record.php', { method: 'POST', body: fd })
            .then(r => r.json())
            .then(res => {
                if (res.success) loadPage(currentPage);
                else alert('Gagal menghapus: ' + res.message);
            });
    }

    searchBtn.addEventListener('click', () => {
        currentSearch = searchInput.value.trim();
        loadPage(1);
    });

    searchInput.addEventListener('keydown', e => {
        if (e.key === 'Enter') {
            currentSearch = searchInput.value.trim();
            loadPage(1);
        }
    });

    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            searchInput.value = ''; // Mengosongkan text input pencarian
            currentSearch = '';     // Mengosongkan variable filter pencarian
            loadPage(1);            // Memuat kembali data halaman pertama dari awal
        });
    }

    loadPage(1);
})();
