#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "time.h"

#include <Adafruit_ADS1X15.h>
#include <EmonLib_ADS1x15.h>

#define ADS (0x48)

EnergyMonitor emon1, emon2;                   // Create an instance

const char* ssid = "mikesnet";
const char* password = "springchicken";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  //Replace with your GMT offset (secs)
const int daylightOffset_sec = 0;   //Replace with your daylight offset (secs)
int hours, mins, secs;
double Irms, Irms2, Irmstot;
float power1, power2;
String power1str = "0";
String power2str = "0";
String strArr[2];

char auth[] = "QY8w6digrc2EItvuiO6AXOmhr88Eiwld";

AsyncWebServer server(80);

WidgetTerminal terminal(V10);

#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))

BLYNK_WRITE(V10) {
  if (String("help") == param.asStr()) {
    terminal.println("==List of available commands:==");
    terminal.println("wifi");
    terminal.println("==End of list.==");
  }
  if (String("wifi") == param.asStr()) {
    terminal.print("Connected to: ");
    terminal.println(ssid);
    terminal.print("IP address:");
    terminal.println(WiFi.localIP());
    terminal.print("Signal strength: ");
    terminal.println(WiFi.RSSI());
    printLocalTime();
  }
    if (String("temps") == param.asStr()) {

    terminal.print("Irms: ");
    terminal.println(Irms);
    
  }
  terminal.flush();
}

void printLocalTime() {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  terminal.println(asctime(timeinfo));
  terminal.flush();
}

void setup(void) {
  Serial.begin(9600);
  emon1.current(ADS, true, GAIN_ONE, RATE_ADS1115_860SPS, 112.31); //22.1ohm
  emon2.current(ADS, true, GAIN_ONE, RATE_ADS1115_860SPS, 113.34);       //21.9ohm   // Current : input ads_address, with init ads, Gain, Data rate, calibration.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  delay(250);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Blynk.config(auth, IPAddress(192, 168, 50, 197), 8080);
  Blynk.connect();
  delay(250);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  hours = timeinfo.tm_hour;
  mins = timeinfo.tm_min;
  secs = timeinfo.tm_sec;
  terminal.println("***SERVER STARTED***");
  terminal.print("Connected to ");
  terminal.println(ssid);
  terminal.print("IP address: ");
  terminal.println(WiFi.localIP());
  printLocalTime();
  terminal.flush();
}

void loop() {
      Blynk.run();
       Irms = emon1.calcIrms(0, 1480);  // Calculate Irms only from Single A0 pin. , NUMBER_OF_SAMPLES
       Irms2 = emon2.calcIrms(1, 1480);
       Irmstot = Irms + Irms2;
      // double Irms = emon1.calcIrms(32, 1480);  // Calculate Irms only from Differential A2-A3 pins. NUMBER_OF_SAMPLES
      every(1000){
        if (millis() > 20000) {Blynk.virtualWrite(V3, Irmstot);
        Blynk.virtualWrite(V2, (Irmstot*120.0));}
      }

   

}
