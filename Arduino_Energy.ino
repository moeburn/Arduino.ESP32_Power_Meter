
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1, emon2;                   // Create an instance

void setup()
{  
  Serial.begin(9600);
  
  emon1.current(A0, 60.6);             // Current: input pin, calibration.
  emon2.current(A1, 60.6);
}

void loop()
{
  double Irms = emon1.calcIrms(1676);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(1676);
      int Irms_i = int((Irms + 0.005) * 100); // ADD 0.005 FOR * 100
      int Irms2_i = int((Irms2 + 0.005) * 100); // ADD 0.005 FOR * 100
      char buffer[11];
      snprintf(buffer, 10, "%d,%d", temp_i, humi_i);
      Serial.println(buffer);
}