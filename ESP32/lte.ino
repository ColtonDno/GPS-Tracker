void disconnect()
{
  // Shutdown
  http.stop();
  SerialMon.println(F("Server disconnected"));

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
}

void httpsPOST()
{
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  SerialMon.print(F(": "));

  modem.gprsConnect(apn, gprsUser, gprsPass);
  
  while (!modem.isGprsConnected())
  {
    SerialMon.println(" failed");
    delay(5000);
    Serial.println("Attempting to reconnect: ");
    modem.gprsConnect(apn, gprsUser, gprsPass);
  }
  SerialMon.println(" success");

  SerialMon.print(F("Performing HTTPS POST request... "));
  // http.connectionKeepAlive();  // Currently, this is needed for HTTPS
  String timestamp = String(year) + "-" + String(month) + "-" + String(day) + " " + String(hour) + ":" + String(minute) + ":" + String(second) + ".000000";
  String postRequestData = "api_key=" + api_key + "&device_id=" + device_id + "&latitude=" + String(lat, 6) + "&longitude=" + String(lon,6) + "&altitude=" + String(alt,4) + "&accuracy=" + String(accuracy,4) + "&timestamp=" + timestamp + "";
  Serial.println(postRequestData);
  Serial.println();
  String contentType = "application/x-www-form-urlencoded";

  int res = http.post(resource, contentType, postRequestData);
  if (res != 0)
  {
    Serial.print("POST resquest failed - ");
    Serial.println(res);
    disconnect();
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print(F("Response status code: "));
  SerialMon.println(status);
  if (!status) 
  {
    disconnect();
    return;
  }

  SerialMon.println(F("Response Headers:"));
  while (http.headerAvailable()) 
  {
    String headerName  = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) 
  {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }

  if (http.isResponseChunked())
    SerialMon.println(F("The response is chunked"));

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  disconnect();
}