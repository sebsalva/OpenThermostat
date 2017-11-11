// ------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <Time.h>

#include "Application.h"
#include "IRManager.h"
#include "WifiManager.h"
#include "SensorManager.h"
#include "Thermostat.h"
#include "Web.h"
#include "TimeNtp.h"
#include "DomoticzBroadcaster.h"
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// Global var
// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DHTTYPE DHT22 // DÃ©finition du type de capteur de tempÃ©rature / humiditÃ©

//CMD
#define CMD_REBOOT                          111
#define CMD_WIFI_DISCONNECT                 222
#define CMD_WEB_DISCONNECT                  333

//timers
unsigned long timerth=0.0;
unsigned long timer60=0.0;
unsigned long timer1=0.0;
uint8_t loggingloop=0;

//Objects
Application config("f6.txt");
TimeNtp timentp;
WifiManager WifMan(&config);
DomoticzBroadcaster domo(&config); 
SensorManager sensorMan(&config, &domo);
IRManager  irmanager(&config);
Thermostat thermos("TCal.txt","Tconfig.txt",&config,&sensorMan,&irmanager,&domo);
Web web(&config,&sensorMan,&thermos,&WifMan,&irmanager,&timentp, &domo);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// Setup et Loop
// ------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
ESP.eraseConfig();
Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
while (!Serial) ; // wait for Arduino Serial Monitor
config.init();
sensorMan.init();
WifMan.initWifi();
timentp.init(config.Ntp,config.Timezone);
irmanager.init();
thermos.init();
web.init();
}

void runOnesecond()
{
  //update next timer
  timer1 = millis() + 1000;

// check CMD
if (config.Cmd==CMD_REBOOT)
{
  DEBUG_PRINT(F("Reboot CMD"));
  
  config.AddLog(F("Reboot CMD"));
  web.server.stop();
  web.server.close();
  WifMan.disconnect();
  ESP.restart();
}

//PIR
sensorMan.readPir(thermos.prestime);

}
void runthseconds()
{
loggingloop++;
if (loggingloop==10) config.Access=false; 
  //update next timer
  timerth = millis() + 30000;
// DHT
sensorMan.ReadTempHum();

//thermostat control state
  if (thermos.IsStarted()== 1)
  {
  //check thermos statut with current temperature
  thermos.run();
  }
  
}

void loop() {
//timers  
  if (millis() > timerth)
      runthseconds();

 if (millis() > timer60){
     if (config.domo) domo.sendTemp(sensorMan.getTemp());
      timer60 = millis() + 60000;
 }
 if (millis() > timer1)
 runOnesecond();
 
//everyloop web
web.server.handleClient();
Alarm.delay(100); // wait
}


void Repeats(){
// if (DEBUG) Serial.println("10 second timer");    
//  if (DEBUG) Serial.println(hour());     
}



