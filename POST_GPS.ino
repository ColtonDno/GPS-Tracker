#define TINY_GSM_MODEM_SIM7000

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library

#define GSM_BAUD 9600
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true

// set GSM PIN, if any
#define GSM_PIN ""

// Server details
#include "Secrets.h"
const int  port = 80;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

TinyGsmClient client(modem);
HttpClient    http(client, server, port);

#define uS_TO_S_FACTOR      1000000ULL  // Conversion factor for micro seconds to seconds
#define UART_BAUD           9600
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4
#define LED_PIN             13

float latitude = 0, longitude = 0, speed = 0, accuracy = 0;
float timer = millis();

void enableGPS(void)
{
  // Set SIM7000G GPIO4 LOW ,turn on GPS power
  // CMD:AT+SGPIO=0,4,1,1
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,1");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,1 false ");
  }
  modem.enableGPS();
}

void disableGPS(void)
{
  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,0 false ");
  }
  modem.disableGPS();
}

void modemPowerOn()
{
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1000);    //Datasheet Ton mintues = 1S
  digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1500);    //Datasheet Ton mintues = 1.2S
  digitalWrite(PWR_PIN, HIGH);
}

void modemRestart()
{
  modemPowerOff();
  delay(1000);
  modemPowerOn();
}

void setup() {
  SerialMon.begin(115200);
  delay(10);

  SerialMon.println("Wait...");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Set GSM module baud rate
  //TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  modem.setBaud(GSM_BAUD);
  modemPowerOn();
  delay(100);
  
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(6000);

  SerialMon.println("Initializing modem...");
  if (!modem.init()) {
    modemRestart();
    delay(1000);
    Serial.println("Failed to restart modem, attempting to continue without restarting");
    return;
  }

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  SerialMon.println("Finished Setup");
}

void loop() {
  //Doesn't work without this for whatever reason
  if (!modem.gprsConnect("YourAPN", "", "")) {
    delay(1000);
    return;
  }
  if (modem.isGprsConnected()) { SerialMon.println("GPRS connected"); }

  if (!modem.isNetworkConnected()) {
    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
      SerialMon.println(" fail");
      delay(1000);
      return;
    }
    SerialMon.println(" success");
    digitalWrite(LED_PIN, HIGH);
  }

  if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }

  SerialMon.println("Enabling GPS");
  enableGPS();
  
  float alt;
  int year, month, day, hour, minute, second, vsat, usat;
  while (1) {
    if (modem.getGPS(&latitude, &longitude, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
      /*SerialMon.println("The location has been locked:");
      SerialMon.print("latitude: "); SerialMon.println(latitude, 6);
      SerialMon.print("longitude: "); SerialMon.println(longitude, 6);
      SerialMon.print("speed: "); SerialMon.println(speed, 3);
      SerialMon.print("accuracy: "); SerialMon.println(accuracy, 4);*/
      break;
    }
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(2000);
  }
  digitalWrite(LED_PIN, LOW);

  disableGPS();

  delay(1000);

  String httpRequestData = "api_key=" + apiKeyValue + "&latitude=" + String(latitude, 6) + "&longitude=" + String(longitude,6) + "&speed=" + String(speed,3) + "&accuracy=" + String(accuracy,4) + "";
  SerialMon.println(httpRequestData);
  String contentType = "application/x-www-form-urlencoded";
  SerialMon.print(F("Performing HTTP POST request... "));
  int err = http.post(resource, contentType, httpRequestData);
  if (err != 0) {
    SerialMon.println(F("failed to connect"));
    SerialMon.println(err);
    delay(5000);
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print(F("Response status code: "));
  SerialMon.println(status);
  if (!status) {
    delay(5000);
    return;
  }

  SerialMon.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName  = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }
  if (http.isResponseChunked()) {
    SerialMon.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  //Wait until at least a minute has passed before looping
  while (millis() - timer < 60000)
    delay(1000);
  timer = millis();
}