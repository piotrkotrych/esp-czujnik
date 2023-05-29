<?php

//headers to bypass cors
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: *");
header("Access-Control-Allow-Methods: *");

//database connection
$servername = "localhost";
$username = "so718_sensor";
$password = "";
$dbname = "so718_sensor";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
  echo http_response_code(400);
  die("Connection failed: " . $conn->connect_error);
}

//get data from database
$sql = "SELECT * FROM `sensor` ORDER BY `measure_time` DESC LIMIT 1";
$result = $conn->query($sql);

//check if data exists
if ($result->num_rows > 0) {
  // output data of each row
  $row = $result->fetch_assoc();
  echo json_encode($row);
} else {
  echo http_response_code(400);
  die();
}

$conn->close();
