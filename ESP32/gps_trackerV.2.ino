#define TINY_GSM_MODEM_SIM7000

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library

#define SerialMon Serial
#define SerialAT Serial1

// #define DUMP_AT_COMMANDS

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#ifndef TINY_GSM_RX_BUFFER
#define TINY_GSM_RX_BUFFER 650
#endif

#define UART_BAUD           115200

#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define LED_PIN             12

#define TINY_GSM_USE_GPRS true

float lat, lon, speed, alt, accuracy;
int year, month, day, hour, minute, second, vsat, usat;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <SSLClient.h>
#include "certs.h"
#include "secrets.h"

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

TinyGsmClient client(modem);
SSLClient secure_layer(&client);
HttpClient http(secure_layer, hostname, port);

void setup()
{
  SerialMon.begin(115200);
  delay(1000);
  SerialMon.println("Wait...");

  SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);

  pinMode(MODEM_PWRKEY, OUTPUT);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(300); //Delay as per the datasheet
  digitalWrite(MODEM_PWRKEY, LOW);

  pinMode(MODEM_FLIGHT, OUTPUT);
  digitalWrite(MODEM_FLIGHT, HIGH);

  SerialMon.println("Initializing modem...");
  if (!modem.init()) 
    modem.restart();
  delay(1000);

  String modemInfo = modem.getModemInfo();
  while (modemInfo == "")
  {
    Serial.println(F("Communication failed. Restarting modem"));
    modem.restart();
    delay(1000);
    modemInfo = modem.getModemInfo();
  }
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  uint8_t ret = 0;
  while (ret == 0)
  {
    ret = modem.setNetworkMode(2);

    if (ret == 0)
    {
      Serial.println(F("Failed to set network mode"));
      delay(1000);
    }
  }

  while (ret == 0)
  {
    ret = modem.setPreferredMode(1);

    if (ret == 0)
    {
      Serial.println(F("Failed to set preferred mode"));
      delay(1000);
    }
  }

  delay(2000);

  secure_layer.setCACert(root_ca);
}

void loop() 
{
  getGPSData();
  httpsPOST();

  static uint32_t timer = 0;
  while(millis() - timer < 60000);
  timer = millis();
}
