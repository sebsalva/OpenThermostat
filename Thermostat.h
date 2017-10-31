#ifndef Thermostat_h
#define Thermostat_h


#include <TimeAlarms.h>
#include "Application.h"
#include "SensorManager.h"
#include "IRManager.h"
#include "DomoticzBroadcaster.h"

//planning for auto mode
//Planning input (format ST(0,1),HH:MM,CMD(Off,Eco,Comfort,Frostfree),DAY(1,...,7 for Sunday, Monday, ..., Saturday or 8 for weekdays 9 for weekend A for All

class Thermostat
{
private:
static Thermostat* getThermosObject;
static String modeCal;
Application *lconfig;
SensorManager *lsensor;
IRManager *lIR;
static DomoticzBroadcaster * ldomo;
String file;
String fileCal;
void load_Cal();
void Autochecking();
static void globalOff();
static void globalComfort();
static void globalEco();
static void globalFrostfree();
void setAlarm(uint8_t i, char mode, uint8_t nbday,uint8_t h,uint8_t s);

AlarmID_t * AlarmTab=NULL;
String * Cal=NULL;
uint8_t checkConfort = 0;
uint8_t Calidx=0;

public:
float Eco;
float Frostfree;
float Comfort;
float hysteresis; //-- theshold value
float triggerHeat; // -- threshold value for starting/stopping heat pump
uint8_t prestime; //--time during which user is present after detecttion in minutes
bool presentbool; //--special mode if someone is inside and if eco ->change to comfort
uint8_t presmin=8;
uint8_t presmax=20;
Thermostat(String s, String fcal, Application *c, SensorManager *se, IRManager *ir,DomoticzBroadcaster *domo);
Thermostat(Application *c, SensorManager *se, IRManager *ir,DomoticzBroadcaster *domo );
void run();
void off();
void on();
void save();
    void save_Cal(int l);
    void save_Cal(String n);
    void init();
uint8_t  getnbCal() {return Calidx;};
String * getCal() {return Cal;};
uint8_t IsStarted();
};

#endif

