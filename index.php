<?php
require_once 'firebase.php';

// Ambil data dari Firebase
$realTimeData = getRealTimeData();
$sensorData = getSensorData();
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="refresh" content="5"> <!-- Halaman akan reload setiap 10 detik -->
    <title>Monitoring Penyiraman Tanaman</title>
    <!-- Import Font dan CSS -->
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <h1>Monitoring Penyiraman Tanaman Otomatis</h1>

    <div class="container">
        <!-- Data Real-Time -->
        <div class="card">
            <h2>Data Real-Time</h2>
            <?php if (!empty($realTimeData)) { ?>
                <p>Kelembaban Udara: <strong><?php echo htmlspecialchars($realTimeData['Humidity']); ?>%</strong></p>
                <p>Suhu: <strong><?php echo htmlspecialchars($realTimeData['Temperature']); ?>°C</strong></p>
                <p>Kelembaban Tanah: <strong id="soilMoisture"><?php echo htmlspecialchars($realTimeData['SoilMoisture']); ?></strong></p>
                <p>Timestamp: <strong><?php echo htmlspecialchars($realTimeData['Timestamp']); ?></strong></p>
                <p>Status Pompa: 
                <strong id="pumpState" style="color: <?php echo $realTimeData['PumpState'] ? '#4CAF50' : '#f44336'; ?>;">
                 <?php echo $realTimeData['PumpState'] ? "Menyala" : "Mati"; ?>
                </strong>
                </p>
            <?php } else { ?>
                <p>Data Real-Time tidak tersedia.</p>
            <?php } ?>
        </div>

        <!-- Kontrol Pompa -->
      

        <!-- Riwayat Data Sensor -->
        <div class="card">
            <h2>Riwayat Data Sensor</h2>
            <div class="table-container-scrollable">
                <table>
                    <thead>
                        <tr>
                            <th>Timestamp</th>
                            <th>Kelembaban Udara</th>
                            <th>Suhu</th>
                            <th>Kelembaban Tanah</th>
                            <th>Status Pompa</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php if ($sensorData) { 
                            foreach ($sensorData as $key => $data) { ?>
                                <tr>
                                    <td><?php echo $key; ?></td>
                                    <td><?php echo $data['Humidity']; ?>%</td>
                                    <td><?php echo $data['Temperature']; ?>°C</td>
                                    <td><?php echo $data['SoilMoisture']; ?>%</td>
                                    <td><?php echo $data['PumpState'] ? "Menyala" : "Mati"; ?></td>
                        
                                </tr>
                        <?php } 
                        } else { ?>
                            <tr><td colspan="5">Data Riwayat tidak tersedia.</td></tr>
                        <?php } ?>
                    </tbody>
                </table>
            </div>
        </div>
    </div>
</body>
</html>
