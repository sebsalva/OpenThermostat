#ifndef SensorManager_h
#define SensorManager_h

#include <DHT.h>
#include "Application.h"
#include "DomoticzBroadcaster.h"

class SensorManager
{
private:
    Application *lconfig;
    DomoticzBroadcaster * ldomo;
// Initialisation du capteur de tempÃ©rature DHT
  DHT *dht;
  unsigned long previousMillis = 0;        // Stockage de l'heure la derniÃ¨re lecture du capteur de tempÃ©rature / humiditÃ©
 // const long interval = 2000;              // Delai entre deux requÃªtes de lecture du capteur de tempÃ©rature / humiditÃ©
  unsigned long previousMillisPIR=0; // Temps de la derniÃ¨re lecture du capteur de prÃ©sence
uint8_t  inputStatePIR=0;
unsigned long presencefirst=0;
unsigned long presencelast=0;
unsigned int presencecount=0;
float Temp=-1.0;
float Hum=-1.0;

public:
SensorManager(Application *c, DomoticzBroadcaster * domo);
SensorManager();
void init();
void ReadTempHum();
float getDirectTemp();
float getDirectHum();
void readPir(uint8_t prestime);
uint8 getStatePIR();
long getPirlast() {return presencelast;};
long getPirfirst() {return presencefirst;};
unsigned int getPircount() {return presencecount;};
float getTemp() {return Temp;};
float getHum() {return Hum;};
};
#endif

