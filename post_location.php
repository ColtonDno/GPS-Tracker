<?php
$servername = "localhost";

//Database name
$dbname = "";
//Database user
$username = "";
//Database user password
$password = "";

$api_key_value = "";

$api_key = $latitude = $longitude = $speed = $accuracy = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $latitude = test_input($_POST["latitude"]);
        $longitude = test_input($_POST["longitude"]);
        $speed = test_input($_POST["speed"]);
        $accuracy = test_input($_POST["accuracy"]);
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO locationData (latitude, longitude, speed, accuracy)
                VALUES ('" . $latitude . "','" . $longitude . "','"  . $speed . "','"  . $accuracy . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
            //echo "api_key=" . $apiKeyValue . " latitude=" . $latitude . " longitude=" . $longitude . " speed=" . $speed . " accuracy=" . $accuracy . " ";
            echo $sql;
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }
}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}