#include "Application.h"

Application::Application()
{
file="";
Logs=new String[NBLOGS];
}
Application::Application(String s)
{
file=s;  
Cmd=0;
Logs=new String[NBLOGS];
}



void Application::init()
{
  Access=false;
// VÃ©rification de l'existance du fichier, s'il n'est pas prÃ©sent on le crÃ©Ã©
  SPIFFS.begin();
  if (!SPIFFS.exists(file))
  {
    DEBUG_PRINTLN(F("File Creation"));
    File f = SPIFFS.open(file, "w");
    f.print(F("0.0;0.0;ESP8266;esppwd;0;0;0;0;toshiba;fr.pool.ntp.org;2;0.0.0.0;0.0.0.0;0.0.0.0;Off;0.0;0.0;esppwd;0;;-1;-1;-1;-1;"));
    TempE=0.0;
    HumE=0.0;
    Wssid="ESP8266";
    Wpass="esppwd";
    dhtType=0;
    dhtPin=0;
    pirPin=0;
    irPin=0;
    AP="toshiba";
    Ntp="fr.pool.ntp.org";
    Timezone=2;
    Ip="0.0.0.0";
    Gat="0.0.0.0";
    Dns="0.0.0.0";
    Mode="Off";
    Temp=0.0;
    FTemp=0.0;
    pwd="esppwd";
    domo=false;
    domoIP="";
    domoPir=-1;
    domoTemp=-1;
    domoMode=-1;
    domogTemp=-1;
  }
  else load();
      
SPIFFS.end();
}


void Application::load()
{
DEBUG_PRINTLN(F("App config reading"));
SPIFFS.begin();
File f = SPIFFS.open(file, "r");
TempE = f.readStringUntil(';').toFloat();
HumE = f.readStringUntil(';').toFloat();
Wssid=f.readStringUntil(';');
Wpass=f.readStringUntil(';');
dhtType=f.readStringUntil(';').toInt();
dhtPin=f.readStringUntil(';').toInt();
pirPin=f.readStringUntil(';').toInt();
irPin=f.readStringUntil(';').toInt();
AP=f.readStringUntil(';');      
Ntp=f.readStringUntil(';');
Timezone=f.readStringUntil(';').toInt();
Ip=f.readStringUntil(';');
Gat=f.readStringUntil(';');
Dns=f.readStringUntil(';');
Mode=f.readStringUntil(';');      
Temp = f.readStringUntil(';').toFloat();
FTemp = f.readStringUntil(';').toFloat();
pwd=f.readStringUntil(';');
if (f.readStringUntil(';').toInt()==0) domo=false;
else domo=true;
domoIP=f.readStringUntil(';');
domoPir=f.readStringUntil(';').toInt();
domoTemp=f.readStringUntil(';').toInt();
domoMode=f.readStringUntil(';').toInt();
domogTemp=f.readStringUntil(';').toInt();
f.close();
SPIFFS.end();
}

String Application::getfile()
{
return file;   
//Serial.println("Save config : "+chaineFormatee);
}
void Application::save()
{
String chaineFormatee =getfile();
DEBUG_PRINTLN("Save config : "+chaineFormatee);
SPIFFS.begin();
SPIFFS.remove(file); // Supression de l'ancien fichier pour le remplacer par le nouveau contenant les nouvelles valeurs
File f = SPIFFS.open(file, "w"); // Ouverture du fichier
f.print(chaineFormatee); // Ecriture dans le fichier
f.close();
SPIFFS.end();
}

//addto logs

void Application::AddLog(String s)
{
uint8_t end= (nblogs+firstlog) % NBLOGS;

Logs[end]=String(hour())+":"+String(minute())+":"+String(second())+" "+s;
if (nblogs < NBLOGS) nblogs++;
if (end == firstlog)
         firstlog = (firstlog + 1) % NBLOGS;
}



