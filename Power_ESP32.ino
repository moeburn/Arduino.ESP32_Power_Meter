#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "time.h"
#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2


 const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;
boolean seenData = false;


               // Create an instance

const char* ssid = "mikesnet";
const char* password = "springchicken";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  //Replace with your GMT offset (secs)
const int daylightOffset_sec = 0;   //Replace with your daylight offset (secs)
int hours, mins, secs;
double Irms, Irms2, amps1, amps2, Irmstot;
float power1, power2;
String power1str = "0";
String power2str = "0";
String strArr[2];

char auth[] = "QY8w6digrc2EItvuiO6AXOmhr88Eiwld";
char remoteAuth2[] = "8_-CN2rm4ki9P3i_NkPhxIbCiKd5RXhK"; //hubert clock auth

AsyncWebServer server(80);

WidgetTerminal terminal(V10);
WidgetBridge bridge2(V60);

#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))

BLYNK_CONNECTED() {
  bridge2.setAuthToken (remoteAuth2);
}

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
    terminal.print(power1);
    terminal.print("Irms2: ");
    terminal.println(power2);
    
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

void recvWithEndMarker()
{
   static byte ndx = 0;
   char endMarker = '\n';
   char rc;

   while (SerialPort.available() > 0 && newData == false)
   {
      rc = SerialPort.read();

      if (rc != endMarker)
      {
         receivedChars[ndx] = rc;
         ndx++;
         if (ndx >= numChars)
         {
            ndx = numChars - 1;
         }
      }
      else
      {
         receivedChars[ndx] = '\0'; // terminate the string
         ndx = 0;
         newData = true;
      }
   }
}

void parseData()
{
   if (newData == true)
   {
      char *strings[3]; // an array of pointers to the pieces of the above array after strtok()
      char *ptr = NULL; byte index = 0;
      ptr = strtok(receivedChars, ",");  // delimiters, semicolon
      while (ptr != NULL)
      {
         strings[index] = ptr;
         index++;
         ptr = strtok(NULL, ",");
      }
      /*
      // print all the parts   
      Serial.println("The Pieces separated by strtok()");
      for (int n = 0; n < index; n++)
      {
         Serial.print("piece ");
         Serial.print(n);
         Serial.print(" = ");
         Serial.println(strings[n]);
      }
      */
      // convert string data to numbers
      power1 = atof(strings[0]) / 100;
      power2 = atof(strings[1]) / 100;
      newData = false;
   }
}

void setup(void) {
   SerialPort.begin(9600, SERIAL_8N1, 16, 17); 
     //21.9ohm   // Current : input ads_address, with init ads, Gain, Data rate, calibration.
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

   recvWithEndMarker();
   parseData();

       Irmstot = power1 + power2;
      // double Irms = emon1.calcIrms(32, 1480);  // Calculate Irms only from Differential A2-A3 pins. NUMBER_OF_SAMPLES
      every(2000){

        Blynk.virtualWrite(V3, Irmstot);
        Blynk.virtualWrite(V2, (Irmstot*120.0));
        Blynk.virtualWrite(V4, power1);
        Blynk.virtualWrite(V5, power2);
        
      }
      every(10000){
        bridge2.virtualWrite(V81, Irmstot);
      }
}
