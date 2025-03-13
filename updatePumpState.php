<?php
require_once 'firebase.php';

// Ambil data dari request
$pumpState = isset($_GET['state']) ? $_GET['state'] : false;

// Update status Pompa di Firebase
updatePumpStateInFirebase($pumpState); // Fungsi untuk update status di Firebase

// Fungsi update status Pompa
function updatePumpStateInFirebase($state) {
    // Update Firebase dengan status pompa (true = menyala, false = mati)
    $firebase = new Firebase(); // Misalkan Anda punya kelas Firebase
    $firebase->set('RealTime/PumpState', $state);
}

// Kembalikan respons sukses
echo json_encode(['status' => 'success']);
exit;
?>
