<?php
require("../api/keys.php");

if ($_SERVER["REQUEST_METHOD"] == "POST")
{
    $api_key = format_input($_POST['api_key']);
    if($api_key == $api_key_value) 
    {
        $device_id = format_input($_POST["device_id"]);
        $latitude = format_input($_POST["latitude"]);
        $longitude = format_input($_POST["longitude"]);
        $altitude = format_input($_POST["altitude"]);
        $accuracy = format_input($_POST["accuracy"]);
        $timestamp = "'" . $_POST["timestamp"] . "'";
        
        $conn = pg_connect($conn_request);
        $result = pg_query($conn,
           "set SEARCH_PATH to s_gps_data;
            insert into t_car_tracker(f_device_id, f_latitude, f_longitude, f_speed, f_altitude, f_accuracy, f_timestamp)
            values (". $device_id .",". $latitude .",". $longitude .",0,". $altitude .",". $accuracy .",". $timestamp .")
            ");
            
        echo "POST was successful";
    }
    else
    {
        echo "Wrong API Key provided.";
    }

}
else
{
    echo "No data posted with HTTP POST.";
}

function format_input($data) 
{
    if ($data == null)
        return $data;
    
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
?>

