<?php
date_default_timezone_set('Asia/Bangkok');
require 'connection.php';  // Include database connection
error_reporting(error_reporting() & ~E_NOTICE);

// Check if the "fingerprintID" value has been passed in the GET request
if (isset($_GET["fingerprintID"])) {
    
    $fingerprintID = $_GET["fingerprintID"];  // Get the fingerprint ID from the GET request
    
    // Query to check if the fingerprint ID exists in the table (replace "idf" with your table name)
    $sql = "SELECT * FROM idf WHERE fingerprint_id = '$fingerprintID'";
    $result = mysqli_query($conn, $sql);
    
    // Check if a row with this fingerprint ID exists
    if (mysqli_num_rows($result) > 0) {
        echo "authorized";  // Fingerprint found in the database
    } else {
        // If the fingerprint ID is not found, insert it into the database
        $insertSQL = "INSERT INTO idf (fingerprint_id, user_name) VALUES ('$fingerprintID', 'Unknown User')";
        if (mysqli_query($conn, $insertSQL)) {
            echo "New fingerprint ID inserted successfully. Authorization pending.";
        } else {
            echo "Error inserting fingerprint ID: " . mysqli_error($conn);
        }
    }
    
} else {
    echo "No fingerprint data received";  // Handle cases where no fingerprint ID is sent
}
?>
