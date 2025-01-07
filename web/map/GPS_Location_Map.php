<?php
require("../api/keys.php");

$num_points = 25;
$hide_location = true;
$draw_bounding_circle = false;

$multi_marker = true;
if ($num_points == 1)
    $multi_marker = false;

$conn = pg_connect($conn_request);

$sql = "SELECT f_latitude, f_longitude FROM s_gps_data.t_car_tracker ORDER BY f_id_pk DESC";
if ($num_points != 0)
    $sql = $sql . " LIMIT ". $num_points;
$sql = $sql . ";";

$rand_offset = 0;
if ($hide_location)
    while ($rand_offset == 0)
        $rand_offset = rand(-20,18);

$latitudes = array();
$longitudes = array();
if ($result = pg_query($conn,$sql)) 
{
    if ($num_points == 0)
        $num_points = pg_num_rows($result);
    for($i = 0; $i < $num_points; $i++) //. Base this on the query limit
    {
        $row = pg_fetch_assoc($result);
        $latitudes[] = round($row['f_latitude'],6);
        $longitudes[] = round($row['f_longitude'],6);
        
        // echo "<script>console.log(".$latitudes[$i].",".$longitudes[$i].");</script>";
        
        if($latitudes[$i] == 0)
        {
            unset($latitudes[$i]);
            unset($longitudes[$i]);
            continue;
        }
        $latitudes[$i] += $rand_offset;
        $longitudes[$i] += $rand_offset;
    }
}
else
    echo "Fail";
    
$latitudes = array_reverse($latitudes);
$longitudes = array_reverse($longitudes);

if (count($latitudes) < $num_points)
    $num_points = count($latitudes);

pg_free_result($result);
pg_close($conn);

$lat_center = $lon_center = 0;
$lat_max = $lon_max = -100;
$lat_min = $lon_min = 100;

$locations = array();//'Element type', latitude, longitude, element number, color
for($i = 0; $i < $num_points; $i++)
{
    if($latitudes[$i] == 0)
    {
        echo "<p>" . $latitudes[$i] . "</p>";
        continue;
    }
    $rgb_hex = mapRGB($num_points, $i);
    $locations[] = array('Marker',$latitudes[$i],$longitudes[$i],($i+1),$rgb_hex);
   
    $lat_center += $latitudes[$i];
    $lon_center +=$longitudes[$i];
        
}
$lat_center /= $num_points;
$lon_center /= $num_points;



$dist_from_center = array();
for($i = 0; $i < $num_points; $i++)
    $dist_from_center[] = sqrt( pow( ($latitudes[$i] - $lat_center), 2) + pow( ($longitudes[$i] - $lon_center), 2) );

$radius = max($dist_from_center) * 101139;

function mapRGB(int $max, int $x)
{
    // Use 1275 for full rgb range. 1147 stops at purple
    $rgb_value = round($x  * 1176 / $max);
    $rgb_zone = floor($rgb_value * (1176/255) / 1176);
  
    $r = "00"; $g = "00"; $b = "00";
    
    switch($rgb_zone)
    {
      case(0):
        $r = "ff";
        if ($rgb_value % 255 != 0)
          $g = dechex($rgb_value % 255);
        break;
        
      case(1):
        $g = "ff";
        if (255 - ($rgb_value % 255) != 0)
          $r = dechex(255 - ($rgb_value % 255));
        break;
        
      case(2):
        $g = "ff";
        if ($rgb_value % 255 != 0)
          $b = dechex($rgb_value % 255);
        break;
        
      case(3):
        $b = "ff";
        if (255 - ($rgb_value % 255) != 0)
          $g = dechex(255 - ($rgb_value % 255));
        break;
        
      case(4):
        $b = "ff";
        if ($rgb_value % 255 != 0)
          $r = dechex($rgb_value % 255);
        break;
    }
    
    if (strlen($r) == 1)
        $r = "0" . $r;
    if (strlen($g) == 1)
        $g = "0" . $g;
    if (strlen($b) == 1)
        $b = "0" . $b;
    
    return '#' . $r . $g . $b;
}
?>

<script>
async function initMap() 
{
    const { Map, InfoWindow } = await google.maps.importLibrary("maps");
    const {AdvancedMarkerElement, PinElement} = await google.maps.importLibrary("marker");
    const {event} = await google.maps.importLibrary("core");
    
    var center_point = { lat: <?php echo $lat_center?>, lng: <?php echo $lon_center?>};
          
    var map = new Map(document.getElementById('map'), {
        zoom: 15,
        center: center_point,
        mapId: "a46c01c8ae97639b",
        //mapTypeId: google.maps.MapTypeId.ROADMAP
    });
    
    let hide_location = new Boolean(<?php echo $hide_location?>);
    let multi_marker = new Boolean(<?php echo $multi_marker?>);
    
    function useMultipleMarkers()
    {
        var locations = <?php echo json_encode($locations, JSON_NUMERIC_CHECK)?>;
        // console.log("Locations:");
        // console.log(locations);
            
        
        
        var marker, i, pin, infowindow, openInfoWindow;
            
        for (i = 0; i < locations.length; i++) 
        {  
            pin = new PinElement(
            {
                glyphColor: "#000000",
                background: locations[i][4],
                borderColor: locations[i][4],
                glyph: locations[i][3].toString(),
            });
            // console.log(locations[i][3] + ": " + locations[i][4]);
            
            marker = new AdvancedMarkerElement(
            {
                position: new google.maps.LatLng(locations[i][1], locations[i][2]),
                map: map,
                content: pin.element,
                title: locations[i][3].toString(),
                zIndex: locations[i][3],
            });
            
            
            if (hide_location == true)
                continue;
            
            infowindow = new InfoWindow(
            {
                headerDisabled: true,
            });
            
            var content = '<p style="color:black; font-size:medium;">' + "(" + locations[i][1] + ", " + locations[i][2] + ")" + "</p>";
             
            //https://stackoverflow.com/questions/11106671/google-maps-api-multiple-markers-with-infowindows
            marker.addListener('click', (function(marker, content, infowindow)
            {
                return function() // This uses the js function closure feature
                {
                    if (openInfoWindow)
                        openInfoWindow.close();
                    
                    infowindow.setContent(content);
                    openInfoWindow = infowindow;
                    infowindow.open(map, marker);
                };
            })(marker, content, infowindow));// This gives context to the closure
            
            
            map.addListener('click', () => 
            {
                if (openInfoWindow)
                    openInfoWindow.close();
            });
        }
        
        var bounding_circle = new Boolean(<?php echo $draw_bounding_circle?>);
        if (bounding_circle == true)
        {
            const circle = new google.maps.Circle(//. Convert to library call like marker
            {
                strokeColor: "#FF0000",
                strokeOpacity: 0.8,
                strokeWeight: 2,
                fillColor: "#FF0000",
                fillOpacity: 0.35,
                map,
                center: center_point,
                radius: <?php echo $radius?>,
            });
        }
    }
    
    function useSingleMarker()
    {
        const pin = new PinElement(
        {
            glyphColor: "#9cdcfe",
            background: "#202a8c",
            borderColor: "#202a8c",
        });
        
        const marker = new AdvancedMarkerElement(
        {
            position: center_point,
            map: map,
            content: pin.element,
        });
            
        google.maps.event.addListener(marker, "click", (event) => //. Convert to library call like marker
        {
            window.open("https://www.google.com/maps/search/?api=1&query=<?php echo $lat_center?>,<?php echo $lon_center?>", '_blank');
        });
            
        const circle = new google.maps.Circle(//. Convert to library call like marker
        {
            strokeColor: "#FF0000",
            strokeOpacity: 0.8,
            strokeWeight: 2,
            fillColor: "#FF0000",
            fillOpacity: 0.35,
            map,
            center: center_point,
            radius: <?php echo $radius?>,
        });
    }
    
    if (multi_marker == true)
        useMultipleMarkers();
    else
        useSingleMarker();
}

window.initMap = initMap;
</script>