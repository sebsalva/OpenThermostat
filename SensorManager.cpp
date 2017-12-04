#include "SensorManager.h"

SensorManager::SensorManager()
{
// Initialisation du capteur de tempÃ©rature DHT
lconfig=new Application();
if (lconfig->dhtPin != -1) dht=new DHT(lconfig->dhtPin, DHT22);
}

SensorManager::SensorManager(Application *c, DomoticzBroadcaster * domo)
{
lconfig=c;
ldomo=domo;
// Initialisation du capteur de tempÃ©rature DHT
//dht=new DHT(lconfig->dhtPin, DHT22);
}

//getter
uint8_t SensorManager::getStatePIR() 
{return inputStatePIR; }


void SensorManager::init()
{
Temp=-1.0;
Hum=-1.0;
if (lconfig->dhtPin != -1) {
  dht=new DHT(lconfig->dhtPin, DHT22);
  dht->begin();           // Initialiser le capteur DHT
//pinMode(LED_BUILTIN, OUTPUT); // DÃ©finition de la LED d'affichage
pinMode(lconfig->pirPin, INPUT);
}
}


// Retourne la chaine formatÃ©e de la tempÃ©rature et de l'humiditÃ©
void SensorManager::ReadTempHum() {
 float humidity, temp;  // Valeurs lues par la sonde
  if (lconfig->dhtPin == -1) return;
  // Lecture du capteur possible toutes les deux secondes
  unsigned long currentMillis = millis();

  // Si le dÃ©lai entre deux lectures ont Ã©tÃ© respectÃ©es alors on lis la tempÃ©rature
 // if (currentMillis - previousMillis >= interval) {
   // previousMillis = currentMillis;

    humidity = dht->readHumidity(); // Lecture de l'humiditÃ© (pourcentage)
    temp = dht->readTemperature(); // Lecture de la tempÃ©rature (Â°C)
    // VÃ©rification de la lecture : si les valeurs humidity et temp ne sont pas nes nombres alors on retourne une erreur
    if (isnan(humidity) || isnan(temp)) {
      DEBUG_PRINTLN(F("Error reading DHT"));
    }
    else // Sinon on formatte la chaÃ®ne Ã  retourner et Ã  afficher
    {
       Temp=temp+lconfig->TempE;
       Hum=humidity+lconfig->HumE;
    }
}

// Retourne la chaine formatÃ©e de la tempÃ©rature et de l'humiditÃ©
float SensorManager::getDirectTemp() {
float temp;  // Valeurs lues par la sonde
if (lconfig->dhtPin == -1) return 0.0;
    temp = dht->readTemperature(); // Lecture de la tempÃ©rature (Â°C)
    // VÃ©rification de la lecture : si les valeurs humidity et temp ne sont pas nes nombres alors on retourne une erreur
    if (isnan(temp)) {
      return -1;
    }
    else // Sinon on formatte la chaÃ®ne Ã  retourner et Ã  afficher
    {
      return temp;
    }
  }


// Retourne la chaine formatÃ©e de l'humiditÃ©
float SensorManager::getDirectHum() {
float humidity;  // Valeurs lues par la sonde
if (lconfig->dhtPin == -1) return 0.0;
    humidity = dht->readHumidity(); // Lecture de l'humiditÃ© (pourcentage)
    // VÃ©rification de la lecture : si les valeurs humidity et temp ne sont pas nes nombres alors on retourne une erreur
    if (isnan(humidity)) {
      return -1;
    }
    else // Sinon on formatte la chaÃ®ne Ã  retourner et Ã  afficher
    {
      return humidity;
    }
 }


// Lecture du capteur de prÃ©sence
void SensorManager::readPir(uint8_t prestime)
{
long currentMillis = millis();
uint8_t p;
if (lconfig->pirPin == -1) return;
// Si le dÃ©lai entre la derniÃ¨re et la nouvelle lecture a Ã©tÃ© respectÃ© ou que le capteur est Ã  zero (pas de mouvement) alors on lis le capteur
  // Le delai permets de laisser le temps au potentiel client de lire qu'un mouvement est dÃ©tectÃ© par le capteur
 if (currentMillis - previousMillisPIR >= 5000 || inputStatePIR == 0) {
   
    // Lecture du capteur
  p=digitalRead(lconfig->pirPin);
  if (lconfig->domo && inputStatePIR==1 && p==0)
    ldomo->sendPir(0);
    inputStatePIR=p;
  if (inputStatePIR==1) {
  previousMillisPIR = currentMillis;
     
        if (lconfig->domo) ldomo->sendPir(1);
        long difference = (currentMillis - presencefirst)/1000;
        long difference2 = (currentMillis - presencelast)/1000;        
        if (difference < 180 or (difference2 <= prestime*60 and presencecount>2)  ) {
            presencecount++;
            presencelast=currentMillis;
            DEBUG_PRINTLN("pir detection nb :" + String(presencecount));
          }
          else{
            presencefirst=currentMillis;
            presencelast=presencefirst;
            presencecount=1;  
            lconfig->AddLog("presence count "+String(presencecount));
          }
  
}
  }
}




