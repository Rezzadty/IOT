<?php
require_once 'config.php';

// Fungsi get untuk mengambil data dari Firebase
function getFirebaseData($node) {
    $url = FIREBASE_URL . "/$node.json";
    $response = file_get_contents($url);
    return json_decode($response, true);
}

// Fungsi untuk mengambil data RealTime
function getRealTimeData() {
    return getFirebaseData("RealTime");
}


// Fungsi untuk mengambil riwayat data SensorData dengan data terbaru di atas
// Fungsi untuk mengambil riwayat data SensorData dengan data terbaru di atas
function getSensorData() {
    $data = getFirebaseData("SensorData");  // Mengambil data dari Firebase

    // Cek apakah data ada dan berupa array
    if (is_array($data) && !empty($data)) {
        // Mengurutkan data berdasarkan key (timestamp) secara terbalik agar data terbaru muncul di atas
        // Mengubah timestamp ke format numerik untuk pengurutan yang benar
        foreach ($data as $key => $value) {
            $timestamp = strtotime($key); // Mengkonversi timestamp string ke format timestamp
            $data[$timestamp] = $value;
            unset($data[$key]);
        }

        // Mengurutkan data berdasarkan timestamp
        krsort($data);
        
        // Menambahkan timestamp kembali ke format yang semula
        $sortedData = [];
        foreach ($data as $timestamp => $value) {
            $sortedData[date("Y-m-d H:i:s", $timestamp)] = $value;
        }
        return $sortedData;  // Kembalikan data yang sudah diurutkan dengan timestamp asli
    }

    // Jika data tidak valid, kembalikan array kosong
    return [];
}


?>
