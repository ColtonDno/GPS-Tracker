<?php 
    require("maps.php");
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
    <script src="https://polyfill.io/v3/polyfill.min.js?features=default"></script>
    <link href="maps.css" rel="stylesheet">
  </head>
  <body>
    <h3 id="title">GPS Location</h3>
    
    <div class="container">
        <div id="map"></div>
    </div>

    <script src="https://maps.googleapis.com/maps/api/js?key=AIzaSyBCH8R76C-emfAJm2lBATDlGQPkR6y1q5Y&callback=initMap&v=weekly" defer></script>
  </body>
</html>