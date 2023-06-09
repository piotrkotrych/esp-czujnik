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

//get post data and decode json to php array
$data = json_decode(file_get_contents('php://input'), true);

//check if data exists
if (!isset($data["chipid"]) || !isset($data["email"]) || !isset($data["name"]) || !isset($data["lokacja"]) || !isset($data["temperature"]) || !isset($data["humidity"])) {
  echo http_response_code(400);
  die();
}

//get data from array
$chipid = $data["chipid"];
$email = $data["email"];
$name = $data["name"];
$location = $data["lokacja"];
$temperature = $data["temperature"];
$humidity = $data["humidity"];
$battery_voltage = $data["batteryVoltage"];
$battery_level = $data["batteryPercentage"];

//insert data to database
$sql = "INSERT INTO `sensor` (`id`, `chipid`, `email`, `name`, `location`, `temperature`, `humidity`, `measure_time`, `battery_voltage`, `battery_level`) VALUES (NULL, '$chipid', '$email', '$name', '$location', '$temperature', '$humidity', CURRENT_TIMESTAMP, '$battery_voltage', '$battery_level');";

if ($conn->query($sql) === TRUE) {
  echo http_response_code(200);
} else {
  echo http_response_code(400);
  die();
}

$conn->close();
