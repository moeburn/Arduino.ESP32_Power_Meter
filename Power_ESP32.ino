#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "time.h"
#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2


  // an array to store the received data

boolean newData = false;
boolean seenData = false;
boolean rolledOver = false;

const byte numChars = 128;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing


long whtot, wattHour1, wattHour2, dailywh, dailywhold, rolledoverwh;
float dailykwh;
               // Create an instance

const char* ssid = "mikesnet";
const char* password = "springchicken";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  //Replace with your GMT offset (secs)
const int daylightOffset_sec = 0;   //Replace with your daylight offset (secs)
int hours, mins, secs;
double Irmstot;
float power1, power2, volts, freq;
float Irms1, Irms2, realPower1, realPower2, apparentPower1, apparentPower2, powerFactor1, powerFactor2;

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
        terminal.print(hours);
        terminal.print(":");
        terminal.println(mins);
  terminal.flush();
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (SerialPort.available() > 0 && newData == false) {
        rc = SerialPort.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void parseData() {      // split the data into its parts
  int buh, smuh;
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    Irms1 = atof(strtokIndx) / 100.0;  // copy it to messageFromPC
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    Irms2 = atof(strtokIndx) / 100.0;     // convert this part to an integer

    strtokIndx = strtok(NULL, ",");
    volts = atof(strtokIndx) / 100.0; 
    
    strtokIndx = strtok(NULL, ",");
    freq = atof(strtokIndx) / 100.0; 
    
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    realPower1 = atoi(strtokIndx);    // convert this part to a float
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    apparentPower1 = atoi(strtokIndx); 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    wattHour1 = atol(strtokIndx); 
 

    strtokIndx = strtok(NULL, ",");
    powerFactor1 = atof(strtokIndx); 

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    realPower2 = atoi(strtokIndx);    // convert this part to a float
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    apparentPower2 = atoi(strtokIndx); 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    wattHour2 = atol(strtokIndx); 


    strtokIndx = strtok(NULL, ",");
    powerFactor2 = atof(strtokIndx); 


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

    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();

        newData = false;
    }

       
      // double Irms = emon1.calcIrms(32, 1480);  // Calculate Irms only from Differential A2-A3 pins. NUMBER_OF_SAMPLES
      every(2000){
        Irmstot = realPower1 + realPower2;
        whtot = wattHour1 + wattHour2;
        //Blynk.virtualWrite(V3, Irmstot);
        //Blynk.virtualWrite(V2, (Irmstot*120.0));
        Blynk.virtualWrite(V4, Irms1);
        Blynk.virtualWrite(V5, Irms2);
        Blynk.virtualWrite(V6, ((Irmstot)/1000.0));
        Blynk.virtualWrite(V7, volts);
        Blynk.virtualWrite(V8, freq);
        Blynk.virtualWrite(V9, realPower1);
        Blynk.virtualWrite(V18, realPower2);
        Blynk.virtualWrite(V16, apparentPower1);
        Blynk.virtualWrite(V11, apparentPower2);
        Blynk.virtualWrite(V12, wattHour1);
        Blynk.virtualWrite(V13, wattHour2);
        Blynk.virtualWrite(V14, powerFactor1);
        Blynk.virtualWrite(V15, powerFactor2);
        Blynk.virtualWrite(V17, whtot);
        Blynk.virtualWrite(V21, dailywh); 
        Blynk.virtualWrite(V22, dailywhold);
      }
      every(10000){
        bridge2.virtualWrite(V81, Irmstot);

        //if (dailywh < whtot) {dailywh += ((whtot-dailywh)-dailywhold);}
        dailywh = whtot - dailywhold;

        /*else if ((dailywh > whtot) && !rolledOver) {
          dailywh += whtot;
          rolledoverwh = whtot;
          rolledOver = true;
        }

        else if ((dailywh > whtot) && rolledOver) {
          dailywh += (whtot-rolledoverwh);
          rolledoverwh = whtot;
        }*/

        struct tm timeinfo;
        getLocalTime(&timeinfo);
        hours = timeinfo.tm_hour;
        mins = timeinfo.tm_min;    

        if ((hours == 0) && (mins == 0) && (dailywh > 1000)) {
          Blynk.virtualWrite(V23, dailywh);
          dailywhold = dailywh;
          dailykwh = dailywh/1000.0;
          dailywh = 0;
          Blynk.virtualWrite(V24, dailykwh);
        }
      }
}
