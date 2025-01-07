<?php 
    require("../api/keys.php");
    require("map/GPS_Location_Map.php");
?>

<!DOCTYPE html>
<!--
 @license
 Copyright 2019 Google LLC. All Rights Reserved.
 SPDX-License-Identifier: Apache-2.0
-->
<html>
  <head>
    <title>GPS Tracker</title>
    <link href="map/map.css" rel="stylesheet">
  </head>
  <body>
    <h3 id="title">GPS Location</h3>
    
    <div class="container">
        <div id="map"></div>
    </div>

    <script src="<?php echo $google_maps_api_key?>" defer></script>
  </body>
</html>