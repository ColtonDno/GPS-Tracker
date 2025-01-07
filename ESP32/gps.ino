// Set SIM7000G GPIO4 LOW ,turn on GPS power
void enableGPS(void)
{
  modem.sendAT("+SGPIO=0,4,1,1");
  if (modem.waitResponse(10000L) != 1) 
    DBG(" SGPIO=0,4,1,1 false ");
  
  modem.enableGPS();
}

// Set SIM7000G GPIO4 LOW ,turn off GPS power
void disableGPS(void)
{
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) 
    DBG(" SGPIO=0,4,1,0 false ");
  
  modem.disableGPS();
}

void getGPSData()
{
  Serial.println("\nEnabling GPS");
  enableGPS();

  Serial.print("Getting GPS");
  while (!modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second)) 
  {
    Serial.print(".");
    delay(2000);
  }
    
  Serial.println("\n\nThe location has been locked:");
  
  Serial.print(month);
  Serial.print("/"); Serial.print(day - 1);
  Serial.print("/"); Serial.print(year);
  Serial.print(" "); Serial.print(hour + 7);
  Serial.print(":"); Serial.print(minute);
  Serial.print(":"); Serial.println(second);

  Serial.print("latitude: "); Serial.println(lat, 6);
  Serial.print("longitude: "); Serial.println(lon, 6);

  speed = (speed < -9000.00) ? speed : 0;
  Serial.print("speed: "); Serial.println(speed, 3);

  Serial.print("alt: "); Serial.println(alt, 6);
  Serial.print("vsat: "); Serial.println(vsat);
  Serial.print("usat: "); Serial.println(usat);
  Serial.print("accuracy: "); Serial.println(accuracy, 4);
  
  // digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  delay(2000);

  disableGPS();
}