#include "Thermostat.h"

Thermostat* Thermostat::getThermosObject;
String Thermostat::modeCal;
DomoticzBroadcaster * Thermostat::ldomo;
Thermostat::Thermostat(String cal, String s, Application *c, SensorManager *se, IRManager *ir,DomoticzBroadcaster *domo)
{
lconfig=c;
lsensor=se;
lIR=ir;
file=s;
fileCal=cal;
ldomo=domo;
}


uint8_t Thermostat::IsStarted()
{
  return checkConfort;
}
void Thermostat::init()
{
Thermostat::getThermosObject=this; //wrap
uint8_t i,j;

//free alarm ????
if (Cal != NULL) {
  delete(Cal);
  for(i=0;i<Calidx;i++) {Alarm.disable(AlarmTab[i]); Alarm.free(AlarmTab[i]);}
  delete AlarmTab;
}


SPIFFS.begin();
File f = SPIFFS.open(file, "r");
    if (!f)
    {
        DEBUG_PRINTLN(F("File Creation for Thermostat"));
        File f = SPIFFS.open(file, "w");
        f.print("16.0;12.0;19.0;0.5;0.5;30;0;8;20");
        Eco = 16.0;
        Frostfree= 12.0;
        Comfort=19.0;
        hysteresis = 0.5;
        triggerHeat=0.5;
        prestime=30;
        presentbool=false;
        presmin=8;
        presmax=20;
    }
    else {
        DEBUG_PRINTLN(F("File reading for Thermostat"));
        Eco = f.readStringUntil(';').toFloat();
        Frostfree= f.readStringUntil(';').toFloat();
        Comfort=f.readStringUntil(';').toFloat();
        hysteresis = f.readStringUntil(';').toFloat();
        triggerHeat=f.readStringUntil(';').toFloat();
        prestime=f.readStringUntil(';').toInt();
        if (f.readStringUntil(';').toInt()==1) presentbool=true;
        else presentbool=false;
        presmin=f.readStringUntil(';').toInt();
        presmax=f.readStringUntil(';').toInt();
    }
    f.close();
    SPIFFS.end();
    
//file cal
load_Cal();

//alarms 1,10:00,ON,A 
//alarms 1,10:00,ON,DAYOFWEEK 1,...,7 for Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, or Saturday
// in time .h -> enum 0,...,6
//"dayofweek" can be dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, or dowSaturday. 
// create the alarms 
// Alarm.alarmRepeat(8,30,0, MorningAlarm);  // 8:30am every day
// Alarm.alarmRepeat(17,45,0,EveningAlarm);  // 5:45pm every day 
//Alarm.alarmRepeat(dayofweek, hours, minutes, seconds, function); 
//"dayofweek" can be dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, or dowSaturday. 
//1..7 sunday to saturday
//8 week day
//9 week end
//0 ALL

DEBUG_PRINTLN(F("Setting Alarms"));
uint8_t nc=0;
if (Calidx >0) {

        for (i=0;i<Calidx;i++)
    {
    int commaIndex = Cal[i].indexOf(','); 
    int secondcommaIndex = Cal[i].indexOf(',', commaIndex + 1);
    int thirdcommaIndex = Cal[i].indexOf(',', secondcommaIndex + 1);
    String fourthValue = Cal[i].substring(thirdcommaIndex + 1); // To the end of the string
    switch(fourthValue.charAt(0))
      {
        case 'A':
        nc+=7;
      break;
      case '8':
      nc+=5;
      break;
      
      case '9':
      nc+=2;
      break;

      default:
      nc++;
      }}
        AlarmTab =new AlarmID_t[nc]; //malloc(Calidx*sizeof(AlarmID_t*))
        }
        else AlarmTab=NULL;
nc=0;
for (i=0;i<Calidx;i++)
    {
    int commaIndex = Cal[i].indexOf(',');
    String firstValue = Cal[i].substring(0, commaIndex);
    if (firstValue == "1") {
      int hourindex = Cal[i].indexOf(':', commaIndex + 1); 
      int secondcommaIndex = Cal[i].indexOf(',', commaIndex + 1);
      int thirdcommaIndex = Cal[i].indexOf(',', secondcommaIndex + 1);
      String h = Cal[i].substring(commaIndex + 1, hourindex);
      String sec = Cal[i].substring(hourindex + 1,secondcommaIndex);
      String thirdValue = Cal[i].substring(secondcommaIndex + 1, thirdcommaIndex); 
      String fourthValue = Cal[i].substring(thirdcommaIndex + 1); // To the end of the string
      DEBUG_PRINTLN("Alarm set "+h+" "+sec+" "+thirdValue);
      switch(fourthValue.charAt(0))
      {
        case 'A':
        //if (fourthValue.charAt(1)=='L')
      setAlarm(nc++,thirdValue.charAt(0),0,h.toInt(),sec.toInt());
      break;
      case '8':
      for(j=2;j<=6;j++) setAlarm(nc++,thirdValue.charAt(0),j,h.toInt(),sec.toInt());
      break;
      
      case '9':
      setAlarm(nc++,thirdValue.charAt(0),1,h.toInt(),sec.toInt());
      setAlarm(nc++,thirdValue.charAt(0),7,h.toInt(),sec.toInt());
      break;

      default:
      setAlarm(nc++,thirdValue.charAt(0),fourthValue.toInt(),h.toInt(),sec.toInt());
      }
      }
    }   
      
//Alarm.timerRepeat(10, globalOn);

//start thermos if Application state allows it
if(lconfig->Mode=="Auto") {
  Autochecking();
 // if (lconfig->domo) ldomo->sendMode(modeCal);
}
if(lconfig->Mode != "Off")
{
  on();
}
else if (lconfig->domo) ldomo->sendMode(F("Off"));
}

//set alarms wrt the mode
//1..7 sunday to saturday
//0 ALL
void Thermostat::setAlarm(uint8_t i,char mode, uint8_t nbday,uint8_t h,uint8_t s)
{
DEBUG_PRINTLN("ALARM "+String(i)+String("::")+String(nbday)+String(";")+String(h)+String(":")+String(s)+String(";")+String(mode));
  switch(mode)
        {
          case 'O':
          if (nbday==0)
          AlarmTab[i]=Alarm.alarmRepeat(h,s,0, globalOff);
          else  AlarmTab[i]=Alarm.alarmRepeat((timeDayOfWeek_t)nbday,h,s,0, globalOff);
          break;
          
          case 'E':
          if (nbday==0)
          AlarmTab[i]=Alarm.alarmRepeat(h,s,0, globalEco);
          else AlarmTab[i]=Alarm.alarmRepeat((timeDayOfWeek_t)nbday,h,s,0, globalEco);
          break;

          case 'C':
           if (nbday==0)
          AlarmTab[i]=Alarm.alarmRepeat(h,s,0, globalComfort);
          else  AlarmTab[i]=Alarm.alarmRepeat((timeDayOfWeek_t)nbday,h,s,0, globalComfort);
          break;

          case 'F':
           if (nbday==0)
          AlarmTab[i]=Alarm.alarmRepeat(h,s,0, globalFrostfree);
          else  AlarmTab[i]=Alarm.alarmRepeat((timeDayOfWeek_t)nbday,h,s,0, globalFrostfree);
          break;
          
          default:
          DEBUG_PRINTLN(F("Error alarm")); Serial.println("sasa");
        }   
}


//load from file to tabular of strings
void Thermostat::load_Cal()
{
uint8_t  i=0;
    
SPIFFS.begin();
File f = SPIFFS.open(fileCal, "r");
    if (!f)
    {
        DEBUG_PRINTLN(F("File Cal Creation for Thermostat"));
        f = SPIFFS.open(fileCal, "w");
        f.print("0;");
        Cal=NULL;
        Calidx=0;
    }
    else
    {
      DEBUG_PRINTLN(F("File Cal reading for Thermostat"));
      Calidx=f.readStringUntil(';').toInt();
      DEBUG_PRINTLN(Calidx);
      if (Calidx >0) {
        Cal=new String[Calidx];
        for (i=0;i<Calidx;i++){
           Cal[i]=f.readStringUntil(';');
                 DEBUG_PRINTLN(Cal[i]);
                
        }}
        else Cal=NULL;
      
    }
f.close();
SPIFFS.end();
return;
}

//save file to tabular of strings without l line
void Thermostat::save_Cal(int l)
{
    int i=0;
    SPIFFS.begin();
    File f = SPIFFS.open(fileCal, "w");
    f.print(String(Calidx-1)+";");
    for (i=0;i<Calidx;i++)
    {
        if (i!=l)
        f.print(Cal[i]+";");
    }
    //
    f.close();
    SPIFFS.end();
    if (Cal != NULL) delete(Cal);
    load_Cal();
    init();
}

//save file to tabular of strings with ligne n 
void Thermostat::save_Cal(String n)
{
    uint8_t i=0;
    DEBUG_PRINTLN("cmd saved "+n);
    SPIFFS.begin();
    File f = SPIFFS.open(fileCal, "w");
    f.print(String(Calidx+1)+";");
    for (i=0;i<Calidx;i++)
            f.print(Cal[i]+";");
    f.print(n+";");
    f.close();
    SPIFFS.end();
    init();
    //
    if (Cal != NULL) delete(Cal);
    load_Cal();
}

void Thermostat::save()
    {
        String chaineFormatee =String(Eco,1)+";"+ String(Frostfree,1)+";"+ String(Comfort,1)+";"+ String(hysteresis,1)+";"+ String(triggerHeat,1)+";"+ String(prestime)+";";
        if (presentbool) chaineFormatee+="1;"; else chaineFormatee+="0;";
        chaineFormatee+=String(presmin)+";"+String(presmax)+";";
        SPIFFS.begin();
        SPIFFS.remove(file); // Supression de l'ancien fichier pour le remplacer par le nouveau contenant les nouvelles valeurs
        File f = SPIFFS.open(file, "w"); // Ouverture du fichier
        f.print(chaineFormatee); // Ecriture dans le fichier
        f.close();
        SPIFFS.end();
    }
    
//wrap the following for setSyncProvider
void Thermostat::globalEco()
  {
    DEBUG_PRINTLN(F("Calendar Eco"));
  modeCal=F("Eco");
if(ldomo != NULL) ldomo->sendMode(modeCal);      
  }

//wrap the following for setSyncProvider
void Thermostat::globalOff()
  {
   DEBUG_PRINTLN(F("Calendar Off"));    
  modeCal=F("Off");    
if(ldomo != NULL) ldomo->sendMode(modeCal);
  }

//wrap the following for setSyncProvider
void Thermostat::globalComfort()
  {
    DEBUG_PRINTLN(F("Calendar Comfort"));
  modeCal=F("Comfort");    
if(ldomo != NULL) ldomo->sendMode(modeCal);
     
  }

//wrap the following for setSyncProvider
void Thermostat::globalFrostfree()
  {
  DEBUG_PRINTLN(F("Calendar Frostfree"));
  modeCal=F("Frostfree");
  if(ldomo != NULL) ldomo->sendMode(modeCal);    
  }

//main method 
void Thermostat::run()
{
//thermostat state, called every minute

String currentMode="";
unsigned int result=0;
float expectedTemp=0;
float currentTemp=0;0;
unsigned int HPState=0;
float HPTemp=0.0;
long currentMillis;
long difference,difference2;


if (lconfig->domo && lconfig->domogTemp==0)
currentTemp=lsensor->getTemp();
else currentTemp=ldomo->getTemp();
HPState= lIR->getHPState();
HPTemp=lIR->getHPTemp();

//get mode
if (lconfig->Mode=="Auto")
  currentMode=modeCal;
else  
  currentMode=lconfig->Mode;

//get temperature
if (lconfig->Mode=="Forced")
  expectedTemp=lconfig->FTemp;
else {
    switch(currentMode.charAt(0))
{
    case 'E':
        expectedTemp=Eco;
        break;
    case 'F':
        expectedTemp=Frostfree;
        break;
    case 'C':
        expectedTemp=Comfort;
        break;
    default:
        expectedTemp=0;
} 
  }
lconfig->Temp=expectedTemp;

if (currentTemp<=0) return;
//print
DEBUG_PRINTLN("---Thermostat Mode : "+currentMode+"---");
DEBUG_PRINTLN("Current temperature :" + String(currentTemp));
DEBUG_PRINTLN("HeatpumpSate :" + String(HPState));
lconfig->AddLog("---Thermostat Mode: "+currentMode+ " Current Temp:" + String(currentTemp)+ " Exp Temp:" + String(expectedTemp)+ " HeatpumpState :" + String(HPState)+"---");

//--Pir
/*if (devicechanged[presence]=='On') then
        difference = timedifference(uservariables_lastupdate[presencefirst]);
        difference2 = timedifference(uservariables_lastupdate[presencecount]);
        see_logs('pir '.. difference .. ' ' .. difference2);
      if (difference < 180 or (difference2 < prestime*60 and uservariables[presencecount]>2)  ) then
          commandArray['Variable:' .. presencecount] = tostring(tonumber(uservariables[presencecount])+1);
          see_logs("Thermostat Pir: " .. tostring(tonumber(uservariables[presencecount])+1));
          else
          commandArray['Variable:' .. presencecount] = "1";
          commandArray['Variable:' .. presencefirst] = "up";
          see_logs("Thermostat Pir: 1");
          
end
end
--presence*/
if (presentbool==true) {
          long currentMillis = millis();
        difference = (currentMillis - lsensor->getPirlast())/1000;
        difference2 = (currentMillis - lsensor->getPirfirst())/1000;
      if (hour()>=presmin && hour()<=presmax && difference <=prestime*60 && lsensor->getPircount()>2 && currentMode=="Eco") {
          //someone present and eco -> confort
           DEBUG_PRINTLN("Thermostat: mode eco but presence detected since " +String(difference/60) +  " min-> Comfort mode");
        expectedTemp=Comfort;
        lconfig->AddLog("First detection since "+ String(difference2/60));
        lconfig->AddLog("Thermostat: mode eco but presence detected since " +String(difference/60) +  " min-> Comfort mode");
      }
}


if (currentMode != "Off" && HPState==0 && (currentTemp <= expectedTemp - hysteresis)) {
        DEBUG_PRINTLN(F("Thermostat: Heatpump start"));
        lconfig->AddLog(F("Thermostat: Heatpump start"));
        result=lIR->sendIR(lconfig->AP,"1","2","0",String(expectedTemp),"0","0");
        if (result==0) {
          DEBUG_PRINTLN(F("IR error"));
          }
return;
}

if (currentMode == "Off" && HPState==1) {
        DEBUG_PRINTLN(F("Thermostat: Heatpump stop"));
        lconfig->AddLog(F("Thermostat: Heatpump stop"));
        result=lIR->sendIR(lconfig->AP,"0","0","0","0","0","0");
        if (result==0) {
          DEBUG_PRINTLN(F("IR error"));
        }
return;
}


//if real temp > expected -> shutdown //////->not working if planing ?
if (currentTemp >= expectedTemp + triggerHeat) {
    if (HPState==1) {
        DEBUG_PRINTLN(F("Thermostat:  temp difference important, Heatpump shutdown"));
        lconfig->AddLog(F("Thermostat:  temp difference important, Heatpump shutdown"));
        result=lIR->sendIR(lconfig->AP,"0","0","0","0","0","0");
        if (result==0) {
          DEBUG_PRINTLN(F("IR error"));
        }
      return;
      }
}

if (HPState == 1 && (HPTemp > expectedTemp+hysteresis)) {
    //reduce temp heat pump
        DEBUG_PRINTLN(F("Thermostat: reduce temp Heat pump"));
        lconfig->AddLog(F("Thermostat: reduce temp Heat pump"));
        result=lIR->sendIR(lconfig->AP,"1","2","0",String(expectedTemp),"0","0");
        if (result==0) {
          DEBUG_PRINTLN(F("IR error"));
        }
        return;
}

if (HPState == 1 && (HPTemp  < expectedTemp-hysteresis)) { 
    //increase temp heat pump
        DEBUG_PRINTLN(F("Thermostat: increase temp Heat pump"));
        lconfig->AddLog(F("Thermostat: increase temp Heat pump"));

        result=lIR->sendIR(lconfig->AP,"1","2","0",String(expectedTemp),"0","0");
        if (result==0) {
          DEBUG_PRINTLN(F("IR error"));
          }
}
return;
}

void Thermostat::off()
{
  DEBUG_PRINTLN(F("Thermostat Off"));
  lconfig->AddLog(F("Thermostat Off"));
unsigned int HPState= lIR->getHPState();
checkConfort = 0;
    // Eteindre
    if (HPState == 1)
    {
      lIR->sendIR(lconfig->AP, "0", "0", "0","0", "0", "0");
    }
if(lconfig->domo) 
  ldomo->sendMode(F("Off"));

}

void Thermostat::on()
{
checkConfort = 1;
DEBUG_PRINTLN(F("Thermostat On"));
lconfig->AddLog(F("Thermostat On"));
if(lconfig->domo) {
  ldomo->sendMode(lconfig->Mode);
  if (lconfig->Mode=="Auto") ldomo->sendMode(modeCal);
}

//run();

}

void Thermostat::Autochecking(void)
{
 time_t t = now(); // Store the current time in time 
unsigned int i;
unsigned int lasttotal=0;
unsigned int total=0;
unsigned int min= 25*60;
unsigned max = 0;
String cmdmax;
String thirdValue;
String fourthValue;
DEBUG_PRINTLN(F("Auto mode checking"));
Serial.println(F("autochecking"));
Serial.println(Calidx);
if (timeStatus()==timeSet && lconfig->Mode=="Auto")
{
for (i=0;i<Calidx;i++)
    {
      Serial.println(Cal[i]);
    int commaIndex = Cal[i].indexOf(',');
    String firstValue = Cal[i].substring(0, commaIndex);
    if (firstValue == "1") {
      int hourindex = Cal[i].indexOf(':', commaIndex + 1); 
      int secondcommaIndex = Cal[i].indexOf(',', commaIndex + 1);
      int thirdcommaIndex = Cal[i].indexOf(',', secondcommaIndex + 1);
      String h = Cal[i].substring(commaIndex + 1, hourindex);
      String sec = Cal[i].substring(hourindex + 1,secondcommaIndex);
      thirdValue = Cal[i].substring(secondcommaIndex + 1, thirdcommaIndex); 
      fourthValue = Cal[i].substring(thirdcommaIndex + 1); // To the end of the string
      
      if (fourthValue=="A" || weekday(t)==fourthValue.toInt() || (fourthValue=="9" && (weekday(t)==1 || weekday(t)==7 )) || (fourthValue=="8" && (weekday(t)>=2 && weekday(t)<=6 )) ) {
Serial.println(fourthValue);
        total=h.toInt()*60+sec.toInt();
        if (total<= min) min=total;
        if (total >= max) {max=total; cmdmax=thirdValue;}
        Serial.println(total);
        Serial.println(hour(t)*60+minute(t));
        if ( (hour(t)*60+minute(t) >= total) && (total > lasttotal )) {
          modeCal=thirdValue;
          lasttotal=total;
          Serial.println(F("Auto mode, set to ")); 
          Serial.println(modeCal);
          DEBUG_PRINT(F("Auto mode, set to "));
          DEBUG_PRINTLN(modeCal);
        }
            
      }
    }
    }   
  if (lasttotal==0 && (hour(t)*60+minute(t)) < min  && cmdmax != "Off") {
     modeCal=cmdmax;
          DEBUG_PRINT(F("Auto mode, set to "));
          DEBUG_PRINTLN(modeCal);  
Serial.println(F("Auto modeb, set to ")); 
          Serial.println(modeCal);
}
}
//if (lconfig->domo) ldomo->sendMode(modeCal);

}



