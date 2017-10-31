#ifndef Web_h
#define Web_h

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "Application.h"
#include "SensorManager.h"
#include "Thermostat.h"
#include "WifiManager.h"
#include "IRManager.h"
#include "TimeNtp.h"
#include "DomoticzBroadcaster.h"

//CMD
#define CMD_REBOOT                          111
#define CMD_WIFI_DISCONNECT                 222
#define CMD_WEB_DISCONNECT                  333


class Web
{
private:
ESP8266HTTPUpdateServer httpUpdater;  // Web server for firmware update

Application *lconfig;
SensorManager *lsensor;
Thermostat * lthermos;
WifiManager * lwifi;
IRManager *lIR;
TimeNtp * ltimentp;
DomoticzBroadcaster *ldomo;
void head(String & str);
void forehead(String & str);
void end(String& str);
void gpio(String &opt,uint8_t gpio);
void model(String &opt,String mdl);
void mode(String &opt,String mode);
void handle_root();
void sensor_print();
void sendir();
void modee();
void wifiCredentials();
void calibrateT();
void calibrateH();
void saveconfig();
void delplanning();
void saveplanning();
void savedomo();
void logging();

//html based
void Add_refresh(String & str);
void Add_table(String & str, const String &);
void Add_form(String & str, const String & url);
void Add_input(String & str, const String & id, const String & label, const String val);
void Add_button(String & str, const String & type, const String & label);
void Add_hrefbutton(String & str, const String & type, const String & label,const String & url);



public:
ESP8266WebServer server;
Web(Application *lconfig, SensorManager *lsensor, Thermostat * lthermos, WifiManager * lwifi, IRManager *ir, TimeNtp * ti,DomoticzBroadcaster *dom);
void init();
void root(Application *c,SensorManager *sen,DomoticzBroadcaster *ldomo,String &str);
String formconfig();
void formconfig(Application *c, Thermostat *t, String &str, String &error);
void sensor(Application *c,SensorManager *sen,String &str);
void formplanning(Thermostat *t,String &str,String &error);
void formlog(Application *lc,String &str);
void formlogging(String &str,String &error);
bool hasAccess (Application *c,String &str);
void formdomo(Application *lc,String &str, String &error);
};

#endif

