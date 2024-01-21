/*

"Maximal" sketch to demonstrate emonLibCM

This demonstrates the use of every API function.
This sketch provides an example of every Application Interface function. 
Many in fact set the default value for the emonTx V3.4, and are therefore
not needed in most cases. If you do need to change a value, the 
Application Interface section of the User Documentation gives full details.

*/

#include <Arduino.h>
#include "emonLibCM.h"
                         //  - needs to be same as emonBase / emonPi and emonGLCD. OEM default is 210
bool recalibrate = false;                                  //  Do not demonstrate the recalibration functions



void setup() 
{  

  Serial.begin(9600);
  //Serial.println("Set baud=115200");
  //Serial.end();
  //Serial.begin(115200);
  EmonLibCM_setAssumedVrms(120.0);  
  EmonLibCM_datalog_period(2);          
  //Serial.println("\nEmonTx v3.4 EmonLibCM Continuous Monitoring Maximal Demo"); 
  //Serial.print("\nAssumed voltage for apparent power calculations when no a.c. is detected: ");  Serial.println(EmonLibCM_getAssumedVrms()); 
  //Serial.print("\nValues will be reported every ");  Serial.print(EmonLibCM_getDatalog_period()); Serial.println(" seconds");
  
  EmonLibCM_SetADC_VChannel(0, 85.67);                    // ADC Input channel, voltage calibration - for Ideal UK Adapter = 268.97 
  EmonLibCM_SetADC_IChannel(1, 57.82, 0.6);                // ADC Input channel, current calibration, phase calibration
  EmonLibCM_SetADC_IChannel(2, 57.82, 2.0);                //  The current channels will be read in this order
             //  16.67 for 100 A : 50 mA c.t. with 120R burden - v.t. leads c.t by ~1 degree

  EmonLibCM_setADC(10, 104);                               // ADC Bits (10 for emonTx & Arduino except Due=12 bits, ADC Duration 104 us for 16 MHz operation)
  EmonLibCM_ADCCal(5.0);                                   // ADC Reference voltage, (3.3 V for emonTx,  5.0 V for Arduino)
  
                         // Assumed voltage when no a.c. detected 
  EmonLibCM_cycles_per_second(60);                         // mains frequency 50Hz, 60Hz
                    // period of readings in seconds - normal value for emoncms.org
  
  EmonLibCM_min_startup_cycles(10);      //10                  // number of cycles to let ADC run before starting first actual measurement

  EmonLibCM_setPulseEnable(false);                          // Enable pulse counting. See the documentation for 2-channel versions of these functions.
                        // Initialise to pulse count to zero

  EmonLibCM_setWattHour(0, 0);                             // Wh counters set to zero
  EmonLibCM_setWattHour(1, 0);
 // EmonLibCM_setWattHour(2, 0);
 // EmonLibCM_setWattHour(3, 0);

               // Max number of sensors, limited by wiring and array size.
  
  EmonLibCM_TemperatureEnable(false); 


  EmonLibCM_Init();                                        // Start continuous monitoring.

}

void loop()             
{


  
  if (EmonLibCM_Ready())   
  {

    //Serial.println(EmonLibCM_acPresent()?"AC present ":"AC missing ");
    //delay(5);



    


   // delay(50);

   // Serial.print(" V=");Serial.print(EmonLibCM_getVrms());
    //Serial.print(" f=");Serial.println(EmonLibCM_getLineFrequency(),2);  
    int v = int((EmonLibCM_getVrms() + 0.005) * 100);
    int f = int((EmonLibCM_getLineFrequency() + 0.005) * 100);

   /* for (byte ch=0; ch<2; ch++)
    {
        Serial.print("Ch ");Serial.print(ch+1);
        Serial.print(" I=");Serial.print(EmonLibCM_getIrms(ch),3);
        Serial.print(" W=");Serial.print(EmonLibCM_getRealPower(ch));
        Serial.print(" VA=");Serial.print(EmonLibCM_getApparentPower(ch));
        Serial.print(" Wh=");Serial.print(EmonLibCM_getWattHour(ch));
        Serial.print(" pf=");Serial.print(EmonLibCM_getPF(ch),4);   
        Serial.println();
        delay(10);
    } */
        int Irms_i = int((EmonLibCM_getIrms(0) + 0.005) * 100); // ADD 0.005 FOR * 100
      int Irms2_i = int((EmonLibCM_getIrms(1) + 0.005) * 100); // ADD 0.005 FOR * 100
      int rp = EmonLibCM_getRealPower(0);
      int va = EmonLibCM_getApparentPower(0);
      long wh = EmonLibCM_getWattHour(0);
      float powerfactor = EmonLibCM_getPF(0);
      int rp2 = EmonLibCM_getRealPower(1);
      int va2 = EmonLibCM_getApparentPower(1);
      long wh2 = EmonLibCM_getWattHour(1);
      float powerfactor2 = EmonLibCM_getPF(1);
      char pf[15];
      char pf2[15];
      dtostrf(powerfactor, 6, 4, pf);
      dtostrf(powerfactor2, 6, 4, pf2);
      char buffer[70];
      snprintf(buffer, 70, "%u,%u,%u,%u,%u,%u,%ld,%s,%u,%u,%ld,%s", Irms_i, Irms2_i, v, f, rp, va, wh, pf, rp2, va2, wh2, pf2);
      //snprintf(buffer, 70, "%d,%d", Irms_i, Irms2_i);
      Serial.print("<");
     Serial.print(buffer);
     Serial.println(">");


  }



}
