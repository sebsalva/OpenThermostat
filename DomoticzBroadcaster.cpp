#include "DomoticzBroadcaster.h"


//using https://github.com/Caerbannog/esphttpclient

DomoticzBroadcaster::DomoticzBroadcaster(Application *c)
{
lconfig=c;
}

float DomoticzBroadcaster::getTemp()
{
//http://192.168.1.2:8080/json.htm?type=devices&filter=temp&rid=2774

HTTPClient http;
String url= F("/json.htm?type=devices&filter=temp&rid=");
url+= String(lconfig->domogTemp);
DEBUG_PRINTLN(F("get Temp from Domoticz"));
http.begin(lconfig->domoIP,8080,url);
int httpCode = http.GET();
if (httpCode != 200) {
  //error
  DEBUG_PRINTLN(F("Error Calling Domoticz Temp"));
  lconfig->AddLog(F("Error Calling Domoticz Temp"));  
http.end();
return -1;}
else {
String payload = http.getString();
const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(36) + 740;
DynamicJsonBuffer JSONBuffer(bufferSize);
JsonObject&  parsed= JSONBuffer.parseObject(payload);
JsonObject& result0 = parsed["result"][0];
http.end();
return result0["Temp"]; ;
}
}



void DomoticzBroadcaster::sendPir(uint8_t state)
{
///json.htm?type=command&param=udevice&idx=16&nvalue=0

HTTPClient http;
String url= F("/json.htm?type=command&param=udevice&idx=");
url+= String(lconfig->domoPir);
url+= F("&nvalue=");
url+= String(state);
DEBUG_PRINTLN(F("send Pir to Domoticz"));
if (state==1)
lconfig->AddLog(F("send Pir 1 to Domoticz"));
else lconfig->AddLog(F("send Pir 0 to Domoticz"));

http.begin(lconfig->domoIP,8080,url);
int httpCode = http.GET();
if (httpCode != 200) {
  //error
  DEBUG_PRINTLN(F("Error Calling Domoticz Pir"));
  lconfig->AddLog(F("Error Calling Domoticz Pir"));
  }
  http.end();
}

void DomoticzBroadcaster::sendTemp(float temp)
{
///json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP

HTTPClient http;
String url= F("/json.htm?type=command&param=udevice&idx=");
url+= String(lconfig->domoTemp);
url+=F("&nvalue=0&svalue=");
url+= String(temp,2);
DEBUG_PRINTLN(F("send Temp to Domoticz"));
lconfig->AddLog(F("send Temp to Domoticz"));
http.begin(lconfig->domoIP,8080,url);
int httpCode = http.GET();
if (httpCode != 200) {
  //error
  DEBUG_PRINTLN(F("Error Calling Domoticz Temp"));
  lconfig->AddLog(F("Error Calling Domoticz Temp"));
  }
http.end();
}

void DomoticzBroadcaster::sendMode(String mode)
{
  uint8_t level=0;
  
/*json.htm?type=command&param=switchlight&idx=IDX&switchcmd=Set%20Level&level=LEVEL
//IDX = id of your device (This number can be found in the devices tab in the column "IDX")
//LEVEL = level of your selector (This number can be found in the edit selectors page, in the column "Level", 0 = Off)
modes = {
        Off=0;
        Auto=10;
        Frostfree=20;
        Eco=30;
        Comfort=40;
        Forced=50;
        }*/
if (lconfig->Mode!="Auto" && lconfig->Mode!=mode) return; 
switch(mode.charAt(0))
        {
          case 'O':
          level=0;
	        break;
          
          case 'A':
          level=10;
          break;

          case 'F':
	        if (mode.charAt(1)=='r')	
            level=20;
	        else level=50;
          break;

          case 'E':
          level=30;
          break;

          case 'C':
          level=40;
          break;
          
	        default:
          level=0;
        }
DEBUG_PRINTLN(F("send Mode to Domoticz"));
lconfig->AddLog(F("send Mode to Domoticz"));

HTTPClient http;
String url= F("/json.htm?type=command&param=switchlight&idx=");
url+= String(lconfig->domoMode);
url+= F("&switchcmd=Set%20Level&level=");
url+= String(level);
http.begin(lconfig->domoIP,8080,url);
int httpCode = http.GET();
if (httpCode != 200) {
//error
DEBUG_PRINTLN(F("Error Calling Domoticz Mode"));
lconfig->AddLog(F("Error Calling Domoticz Mode"));
}
http.end();
}
