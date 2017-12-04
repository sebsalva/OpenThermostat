#include "Web.h"


Web::Web(Application *c,SensorManager *s, Thermostat * t,WifiManager * lw, IRManager *ir, TimeNtp * ti, DomoticzBroadcaster *dom) :
 server(80)
{
 lconfig=c;
 lsensor=s;
 lthermos=t;
 lwifi=lw;
 lIR=ir;
 ltimentp=ti;
 ldomo=dom;
}

void Web::init()
{
  server.begin(); // DÃ©marrage du serveur web
//setup firmware update server
httpUpdater.setup(&server, "/up_firmware", "admin", lconfig->pwd.c_str());
//route declaration
// Initialiser la page de lecture de la tempÃ©rature et de l'humiditÃ©
//error: 'this' was not captured for this lambda function : For the fix, just add the reference capture and it will compile fine 
  server.on("/tempandhumidity", [&]() {
    String r="";
    r=String(lsensor->getTemp(),2)+";"+String(lsensor->getHum(),2);
    server.send(200, "text/plain", r);
  });

 // reboot
  server.on("/reboot",HTTP_GET, [&]() {
    server.send(200, "text/html",F("<html><head><meta http-equiv=\"refresh\" content=\"5;URL=/\"></head><body>Rebooting in 5s</body></html>"));
    lconfig->Cmd=CMD_REBOOT;
  });
   // Initialisation de la page de saisie et lecture de la valeur de l'Ã©talonnage (/etalonnage?value=<valeur>)
  server.on("/calibrateTemp",std::bind(&Web::calibrateT, this));

// Initialisation de la page de saisie et lecture de la valeur de l'Ã©talonnage (/etalonnage?value=<valeur>)
  server.on("/calibrateHum",std::bind(&Web::calibrateH, this));

  // RequÃªte du statut du capteur de prÃ©sence (/presence)
  server.on("/presence", [&]() {
   server.send(200, "text.plain", String(lsensor->getStatePIR()));
  });

  // Envoi d'une trame Infrarouge (/sendIR?model=<modele>&pwr=<pwr>&mode=<mode>&fan=<fan>&temp=<temp>&vair=<vair>&hair=<hair>)
  server.on("/sendIR", std::bind(&Web::sendir, this));
  
  server.on("/on", [&]() {
   lconfig->Mode=F("Eco");
   lthermos->on();
    server.send(200, "text.plain", "Thermostat ON");
  });
  
  server.on("/off", [&]() {
   lthermos->off();
    server.send(200, "text.plain", "Thermostat OFF");
  });
  
  //web pages
  // Initialiser la page racine du service Web
  server.on("/", std::bind(&Web::handle_root, this));

  // Initialiser la page de lecture et de modification des valeurs de connection WiFi (/wificredentials?ssid=<ssid>&pwd=<password>)
  server.on("/wificredentials", std::bind(&Web::wifiCredentials, this));

// Log page
  server.on("/log",HTTP_GET, [&]() {
    String r="";
    formlog(lconfig,r);
    server.send(200, "text/html",r);
  });

  // config page
  server.on("/config",HTTP_GET, [&]() {
    String r="";
    String e="";
    formconfig(lconfig,lthermos, r,e);
    server.send(200, "text/html",r);
  });
  
  //sensor page
  server.on("/sensorprint", std::bind(&Web::sensor_print, this));
  
// formulaire config
  server.on("/config",HTTP_POST, std::bind(&Web::saveconfig, this));

// download config
  server.on("/saveconfigfile",HTTP_GET, std::bind(&Web::saveconfigfile, this));

// upload config
 // server.on("/loadconfigfile",HTTP_POST, std::bind(&Web::loadconfigfile, this));
server.on("/loadconfigfile", HTTP_POST, [&]() {
  server.send(200, "text/plain", "upload\r\n");
  }, std::bind(&Web::loadconfigfile, this));
  
// formulaire mode racine
  server.on("/mode",HTTP_POST, std::bind(&Web::modee, this));

// Initialiser la page de config planning
server.on("/planning",HTTP_GET, [&]() {
String r="";
String e="";
formplanning(lthermos, r,e);
server.send(200, "text/html",r);
});

//formulaire Domoticz 
//Dombroadcaster
server.on("/Dombroadcaster",HTTP_GET, [&]() {
String r=""; String e="";
formdomo(lconfig,r,e);
server.send(200, "text/html",r);
});

//Domoticz save
server.on("/savedomo",HTTP_POST, std::bind(&Web::savedomo, this));

//check password form
server.on("/logging",HTTP_POST, std::bind(&Web::logging, this));

//formulaire addplanning
server.on("/addplanning",HTTP_POST, std::bind(&Web::saveplanning, this));

//delete planning
server.on("/delplanning",HTTP_GET, std::bind(&Web::delplanning, this)); //should be HTTP_DELETE !!!!
}

/*web methods called with routes
//
//
*/


// root page
void Web::handle_root() {
bool b=false;
String userVal ="";
String e;

userVal=server.arg("pwd");
if (userVal != lconfig->pwd) // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
if (lconfig->Access == false){
String r="";
if (userVal=="") e=F("Password required");
else e=F("Wrong password");
formlogging(r,e);
server.send(200, "text/html",r);
return;
}}
else {
  lconfig->Access=true;}
  String str="";
  root(lconfig,lsensor,ldomo,str);
  server.send(200, "text/html", str);
}

// sensor page
void Web::sensor_print() {
  String str="";
  sensor(lconfig,lsensor,str);
  server.send(200, "text/html", str);
  //delay(100);
}

void Web::sendir()
{
    String hpmodel = server.arg("model");
    String power = server.arg("pwr");
    String fmode = server.arg("mode");
    String fan = server.arg("fan");
    String temp = server.arg("temp");
    String vair = server.arg("vair");
    String hair = server.arg("hair");
    unsigned int r=lIR->sendIR(hpmodel, power, fmode, fan, temp, vair, hair);
    if (r==0) 
    server.send(200, "text.plain","Done");
    else 
    server.send(200, "text.plain","IR Error");
}

void Web::modee()
{
    String result="";
    String mode = server.arg("mode");
    //String modele = server.arg("model");
    String temp = server.arg("expectedT");
    String ftemp = server.arg("forcedT");
    if (mode == "" || temp  == "" || ftemp  == "" )
      result = F("Call Error");
    else {
      //todo check instead or writing every time
      lconfig->Mode=mode;

//get temperature
if (mode==F("Forced"))
{
  lconfig->Temp=ftemp.toFloat();
  lconfig->FTemp=ftemp.toFloat();
}
else {
  lconfig->FTemp=0.0;
    switch(mode.charAt(0))
{
    case 'E':
        lconfig->Temp=lthermos->Eco;
        break;
    case 'F':
        lconfig->Temp=lthermos->Frostfree;
        break;
    case 'C':
        lconfig->Temp=lthermos->Comfort;
        break;
    default:
        lconfig->Temp=0.0;
} 
  }
      lconfig->save();  
      DEBUG_PRINTLN(F("new config"));
      lconfig->AddLog(F("new config"));
    }
    if (mode == "Off" )
    {
    lthermos->off();
    result = F("Thermos OFF");
    }
    if (mode !="" && mode!="Off" ){
      lconfig->Mode=mode;
      lconfig->FTemp=ftemp.toFloat();
      lthermos->on();
    }
    
  String str="";
  root(lconfig,lsensor,ldomo,str);
  server.send(200, "text/html", str);
}

// Quand l'utilisateur fait appel au service /wificredentials
void Web::wifiCredentials()
{
  String nssid=server.arg("ssid");
  String npwd=server.arg("pwd");
  if (nssid != "") // Si l'utilisateur saisit des valeurs dans l'URL on modifie la connexion Wifi
  {
    if (nssid.indexOf(';') == -1 && npwd.indexOf(';') == -1)
    {
      lconfig->Wssid=nssid;
      lconfig->Wpass=npwd;
      lconfig->save();
      server.stop();
      lwifi->initWifi(); // Initialisation de la connexion WiFi avec les nouvelles donnÃ©es saisies par l'utilisateur
      server.begin();
      server.send(200, "text/plain", "Done");
    }
    else
    {
      server.send(200, "text/plain", "Error : char ';' not allowed");
    }
  }
  else // Sinon on retourne les informations de la connexion actuelle
  {
    String info = lwifi->getWiFiInfo();
    server.send(200, "text/plain", info);
  }
}

// Quand l'utilisateur fait appel au service /calibrateTemp
void Web::calibrateT()
{
  String userVal = server.arg("value");
    
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->TempE=userVal.toFloat();
    lconfig->save();
    server.send(200, "text/plain", "Done");
  }
  else // Sinon on lui affiche la valeur prÃ©sente dans la mÃ©moire
  {
    server.send(200, "text/plain", String(lconfig->TempE));
  }
}
// Quand l'utilisateur fait appel au service /calibrateHum
void Web::calibrateH()
{
  String userVal = server.arg("value");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->HumE=userVal.toFloat();
    lconfig->save();
    server.send(200, "text/plain", "Done");
  }
  else // Sinon on lui affiche la valeur prÃ©sente dans la mÃ©moire
  {
    
    server.send(200, "text/plain", String(lconfig->HumE));
  }
}

void Web::saveconfigfile()
{
  //server.send(200, "text/plain","sending file");
  server.sendHeader("Content-Disposition", "attachment; filename=settings.txt");
  
SPIFFS.begin();
  File file = SPIFFS.open(lconfig->getfile(), "r");                    // Open the file
   size_t sent =  server.streamFile(file, "application/octet-stream" );    // Send it to the client
    file.close();    
    SPIFFS.end();                        
}

void Web::loadconfigfile()
{
HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    SPIFFS.begin();
    DEBUG_PRINTLN(F("handleFileUpload Name: ")); DEBUG_PRINTLN(filename);
    fsUploadFile = SPIFFS.open(lconfig->getfile(), "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();     
      SPIFFS.end();// Close the file again
      DEBUG_PRINTLN(F("handleFileUpload Size: ")); DEBUG_PRINTLN(upload.totalSize);
      server.sendHeader("Location","/reboot");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void Web::saveconfig()
{
   bool b=false;
   String userVal ="";

   userVal=server.arg("spwd");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->pwd=userVal;
  }
   else b=true;
   userVal=server.arg("ssid");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Wssid=userVal;
  }
   else b=true;
  userVal = server.arg("pwd");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Wpass=userVal;
  }
   else b=true;
  userVal = server.arg("ip");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Ip=userVal;
  }
   else b=true;
  userVal = server.arg("gat");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Gat=userVal;
  }
   else b=true;
  userVal = server.arg("dns");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Dns=userVal;
  }
   else b=true;
 userVal = server.arg("ntp");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Ntp=userVal;
  }
   else b=true;
 userVal = server.arg("tzone");
  if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->Timezone=userVal.toInt();
  }
   else b=true;
 
  userVal = server.arg("dhtpin");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->dhtPin=userVal.toInt();
  }
   else b=true;
  userVal = server.arg("dhttype");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->dhtType=userVal.toInt();
  }
   else b=true;
  userVal = server.arg("pirpin");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->pirPin=userVal.toInt();
  }
   else b=true;
  userVal = server.arg("irpin");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->irPin=userVal.toInt();
  }
   else b=true;
   userVal = server.arg("HumE");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->HumE=userVal.toFloat();
  }
   else b=true;
   userVal = server.arg("TempE");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->TempE=userVal.toFloat();
  }
   else b=true;
   userVal = server.arg("model");
   if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  {
    lconfig->AP=userVal;
  }
   else b=true;
//thermostat
userVal = server.arg("ecoT");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->Eco=userVal.toFloat();;
}
else b=true;
userVal = server.arg("frostT");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->Frostfree=userVal.toFloat();
}
else b=true;
userVal = server.arg("ComfortT");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->Comfort=userVal.toFloat();
}
else b=true;
userVal = server.arg("hysteresisT");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->hysteresis=userVal.toFloat();
}
else b=true;
userVal = server.arg("triggerHeatT");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->triggerHeat=userVal.toFloat();
}
else b=true;
userVal = server.arg("prestime");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->prestime=userVal.toInt();
}
else b=true;
userVal = server.arg("presentbool");
if (userVal == "1") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
lthermos->presentbool=true; 
else lthermos->presentbool=false; 
userVal = server.arg("presmin");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->presmin=userVal.toInt();
}
else b=true;
userVal = server.arg("presmax");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->presmax=userVal.toInt();
}
else b=true;
if (b==false) {
lconfig->save();
lthermos->save();
//lsensor->init();
//lIR->init();
//lwifi->initWifi();
//ltimentp->init();
//lthermos->init();
//handle_root();//reboot
server.send(200, "text/html",F("<html><head><meta http-equiv=\"refresh\" content=\"2;URL=/reboot\"></head><body>Ok...</body></html>"));

}
else {
  String r="";
  String e=F("Error in config");
  formconfig(lconfig,lthermos,r,e);
  server.send(200, "text/html",r);   
  }
}

void Web::delplanning()
{
String userVal="";

userVal=server.arg("p");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
lthermos->save_Cal(userVal.toInt());
String r="";
String e="";
formplanning(lthermos, r,e);
server.send(200, "text/html",r);
}
else {
String r="";
String e=F("Error in planning modification");
formplanning(lthermos, r,e);
server.send(200, "text/html",r);
}
}

//called by form planning
void Web::saveplanning()
{
bool b=false;
String userVal ="";
String cmd="";

userVal=server.arg("enabled");
if (userVal == "1") cmd+=F("1,"); 
else cmd+=F("0,");
userVal=server.arg("Thour");
if (userVal != "") cmd+=userVal;
else b=true;
cmd+=F(":");
userVal=server.arg("Tmin");
if (userVal != "") cmd+=userVal;
else b=true;
cmd+=F(",");
userVal=server.arg("command");
if (userVal != "") cmd+=userVal;
else b=true;
cmd+=F(",");
userVal=server.arg("when1");
if (!b && userVal != "") 
	lthermos->save_Cal(cmd+F("A"));
userVal=server.arg("when2");
if (!b && userVal != "") 
lthermos->save_Cal(cmd+userVal);
userVal=server.arg("when3");
if (!b && userVal != "") 
lthermos->save_Cal(cmd+userVal);
userVal=server.arg("when4");
if (!b && userVal != "") {
	userVal=server.arg("ChkMon");
	if (userVal=="1") lthermos->save_Cal(cmd+F("2"));
	userVal=server.arg("ChkTue");
	if (userVal=="1") lthermos->save_Cal(cmd+F("3"));
	userVal=server.arg("ChkWed");
	if (userVal=="1") lthermos->save_Cal(cmd+F("4"));
	userVal=server.arg("ChkThu");
	if (userVal=="1") lthermos->save_Cal(cmd+F("5"));
	userVal=server.arg("ChkFri");
	if (userVal=="1") lthermos->save_Cal(cmd+F("6"));
	userVal=server.arg("ChkSat");
	if (userVal=="1") lthermos->save_Cal(cmd+F("7"));
	userVal=server.arg("ChkSun");
	if (userVal=="1") lthermos->save_Cal(cmd+F("1"));
	}
  DEBUG_PRINTLN(cmd);
if (!b)
	{
	String r="";
	String e="";
	formplanning(lthermos, r,e);
	server.send(200, "text/html",r);
	}
else {
	String r;String e=F("Error in planning");
	formplanning(lthermos, r,e);
	server.send(200, "text/html",r);
	}
}

void Web::savedomo()
{
bool b=false;
String userVal ="";

userVal=server.arg("domo");
if (userVal == "1") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur  
  lconfig->domo=true;
  else lconfig->domo=false;
userVal=server.arg("domoIP");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  lconfig->domoIP=userVal;
  else b=true;
userVal=server.arg("domoPir");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  lconfig->domoPir=userVal.toInt();
  else b=true;
userVal=server.arg("domoTemp");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  lconfig->domoTemp=userVal.toInt();
  else b=true;
  userVal=server.arg("domogTemp");
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  lconfig->domogTemp=userVal.toInt();
  else b=true;
userVal=server.arg("domoMode"); 
if (userVal != "") // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
  lconfig->domoMode=userVal.toInt(); 
  else b=true;

if (b==false) {
lconfig->save();
handle_root();
}
else {
  String r="";
  String e=F("Error in Domoticz config");
  formdomo(lconfig,r,e);
  server.send(200, "text/html",r);   
}
}

void Web::logging()
{
bool b=false;
String userVal ="";

userVal=server.arg("pwd");
if (userVal != lconfig->pwd) // Si l'utilisateur Ã  saisit une valeur alors on Ã©crit dans la mÃ©moire cette valeur
{
String r="";
String e=F("Wrong password");
formlogging(r,e);
server.send(200, "text/html",r);
}
else {
  lconfig->Access=true;
  handle_root();
}
}

/*
 * Methods for creating HTML content
 */


bool Web::hasAccess(Application *c,String & str)
{
if (c->Access== false && c->pwd != "") {
  String error=F("Password required");
 formlogging(str,error);
 return false;
}
 else return true;
}
void Web::forehead(String & str)
{
  str+= F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>OpenThermostat</title><style>/* Copyright 2014 Owen Versteeg; MIT licensed */body,textarea,input,select{background:0;border-radius:0;font:16px sans-serif;margin:0}.smooth{transition:all .2s}.btn,.nav a{text-decoration:none}.container{margin:0 20px;width:auto}label>*{display:inline}form>*{display:block;margin-bottom:10px}.btn{background:#999;border-radius:6px;border:0;color:#fff;cursor:pointer;display:inline-block;margin:2px 0;padding:12px 30px 14px}.btn:hover{background:#888}.btn:active,.btn:focus{background:#777}.btn-a{background:#0ae}.btn-a:hover{background:#09d}.btn-a:active,.btn-a:focus{background:#08b}.btn-b{background:#3c5}.btn-b:hover{background:#2b4}.btn-b:active,.btn-b:focus{background:#2a4}.btn-c{background:#d33}.btn-c:hover{background:#c22}.btn-c:active,.btn-c:focus{background:#b22}.btn-sm{border-radius:4px;padding:10px 14px 11px}.row{margin:1% 0;overflow:auto}.col{float:left}.table,.c12{width:100%}.c11{width:91.66%}.c10{width:83.33%}.c9{width:75%}.c8{width:66.66%}.c7{width:58.33%}.c6{width:50%}.c5{width:41.66%}.c4{width:33.33%}.c3{width:25%}.c2{width:16.66%}.c1{width:8.33%}h1{font-size:3em}.btn,h2{font-size:2em}.ico{font:33px Arial Unicode MS,Lucida Sans Unicode}.addon,.btn-sm,.nav,textarea,input,select{outline:0;font-size:14px}textarea,input,select{padding:8px;border:1px solid #ccc}textarea:focus,input:focus,select:focus{border-color:#5ab}textarea,input[type=text]{-webkit-appearance:none;width:auto}.addon{padding:8px 12px;box-shadow:0 0 0 1px #ccc}.nav,.nav .current,.nav a:hover{background:#000;color:#fff}.nav{height:24px;padding:11px 0 15px}.nav a{color:#aaa;padding-right:1em;position:relative;top:-1px}.nav .pagename{font-size:22px;top:1px}.btn.btn-close{background:#000;float:right;font-size:25px;margin:-54px 7px;display:none}@media(min-width:1310px){.container{margin:auto;width:1270px}}@media(max-width:870px){.row .col{width:100%}}@media(max-width:500px){.btn.btn-close{display:block}.nav{overflow:hidden}.pagename{margin-top:-11px}.nav:active,.nav:focus{height:auto}.nav div:before{background:#000;border-bottom:10px double;border-top:3px solid;content:'';float:right;height:4px;position:relative;right:3px;top:14px;width:20px}.nav a{padding:.5em 0;display:block;width:50%}}.table th,.table td{padding:.5em;text-align:left}.table tbody>:nth-child(2n-1){background:#ddd}.msg{padding:1.5em;background:#def;border-left:5px solid #59d}.hero {background: #eee;padding: 20px;border-radius: 10px;margin-top: 1em;}.hero h1 {margin-top: 0;margin-bottom: 0.3em;}.c4 {padding: 10px;box-sizing: border-box;}.c4 h3 {margin-top: 0;}.c4 a {margin-top: 10px;display: inline-block;}</style></head><body>");
}

void Web::head(String & str)
{
  forehead(str);
  str+=F("<nav class=\"nav\" tabindex=\"-1\" onclick=\"this.focus()\"><div class=\"container\"><a class=\"pagename current\" href=\"/\">Open Thermostat</a><a href=\"/\">Main</a><a href=\"/config\">Config</a><a href=\"/sensorprint\">Sensor State</a><a href=\"/log\">Log&Info</a> Time ");
  str=str+String(hour(),DEC)+F(":")+String(minute(),DEC)+F("</div></nav><div class=\"container\"><div class=\"hero\">");
}

void Web::end(String & str)
{
str+= F("</body></html>"); 
}

void Web::Add_refresh(String & str)
{
str+=F("<script language='JavaScript'>function RefreshMe(){window.location = window.location}setTimeout('RefreshMe()', 3000);</script>");
}

void Web::Add_table(String & str, const String & title)
{
  str+=F("<h2>");
  str+=title;
  str+=F("</h2><table class=\"table\"><tbody><tr>");
}

void Web::Add_form(String & str, const String & url)
{
  str+=F("<form method='POST' action='/");
  str+=url;
  str+=F("'>");
}

void Web::Add_input(String & str, const String & id, const String & label, const String val)
{
str+=F("<td><label for=\"");
str+=id;
str+=F("\">");
str+=label;
str+=F("</label></td><td><input type=\"text\" name=\"");
str+=id;
str+=F("\" id=\"");
str+=id;
str+=F("\" value=\"");
str+=val;
str+=F("\"/></td>");
}

void Web::Add_button(String & str, const String & type, const String & label)
{
str+=F("<input class=\"btn ");
str+=type;
str+=F(" btn-sm smooth\" type='submit' value='");
str+=label;
str+=F("'>");
}

 
void Web::Add_hrefbutton(String & str, const String & type, const String & label,const String & url)
{
str+=F("<a href=\"/");
str+=url;
str+=F("\" class=\"btn btn-sm ");
str+=type;
str+=F(" \">");
str+=label;
str+=F("</a>");
}


void Web::sensor(Application *c,SensorManager *sen,String &str)
{
if (! hasAccess(c,str)) return;
 
head(str);
Add_refresh(str);
Add_table(str, F("Sensor States"));
 str+=F("<td>Temperature</td><td>");
 str+=String(sen->getTemp());
 str+=F("</td></tr><tr><td>Hummidity</td><td>");
 str+=String(sen->getHum());
 str+=F("</td></tr><tr><td>Pir</td><td>");
 str+=String(sen->getStatePIR());
 str+=F("</td></tr></tbody></table></div>");
 end(str);
}

void Web::root(Application *c,SensorManager *sen, DomoticzBroadcaster *ldomo,String &str)
{
if (! hasAccess(c,str)) return;
 head(str);
//Add_refresh(str);
if (c->pwd=="") str+=F("<h3>No password set!</h3>");
 str+=F("<h3>Wifi Info</h3>");
 str+="<p>Wifi SSID: "+ c->Wssid +" </p></br>";
 Add_form(str, F("mode"));
 Add_table(str, F("Thermostat"));
 str+=F("<td><label for=\"mode\">Mode</label></td><td><select name=\"mode\" id=\"mode\">");
 String opt1="";
 mode(opt1,c->Mode);
 str+=opt1;
 str+=F("</select></td></tr>"); 
  str+=F("<tr><td><label>Current Temperature</label></td><td><label>");
 if (c->domo && c->domogTemp!=0) str+=String(ldomo->getTemp());
 else str+=String(sen->getTemp());
 str+=F("</label></td></tr><tr>");
 Add_input(str,F("expectedT"),F("Expected Temperature"),String(c->Temp));
 str+=F("</tr>");
 Add_input(str,F("forcedT"),F("Forced Temperature"),String(c->FTemp));
 str+=F("</tr></tbody></table>");
Add_button(str, F("btn-b"),F("Submit"));
str+=F("</form><h3>Misc</h3>");
Add_hrefbutton(str,F("btn-c"),F("Reboot"),F("reboot"));
str+=F("</div>");
 end(str);
}

void Web::mode(String &opt,String mode)
{
  opt=F(" <option value='Off' "); 
  if(mode=="") opt+=F("selected "); 
  opt+=F(">Off</option><option value='Auto' "); 
  if(mode=="Auto") opt+=F("selected "); 
  opt+=F(">Auto</option><option value='Frostfree' "); 
  if(mode=="Frostfree") opt+=F("selected "); 
  opt+=F(">Frostfree</option><option value='Eco' "); 
  if(mode=="Eco") opt+=F("selected "); 
  opt+=F(">Eco</option><option value='Comfort' "); 
  if(mode=="Comfort") opt+=F("selected "); 
  opt+=F(">Comfort</option><option value='Forced' "); 
  if(mode=="Forced") opt+=F("selected "); 
  opt+=F(">Forced</option>");
}

void Web::model(String &opt,String mdl)
{
  uint8_t i=0;
  String tab[]={F("toshiba"),F("daikin"),F("panasonic_ckp"),F("panasonic_dke"),F("panasonic_jke"),F("panasonic_nke"),F("carrier_mca"),F("carrier_nqv"),F("midea"),F("fujitsu_awyz"),F("mitsubishi_fd"),F("mitsubishi_fe"),
  F("mitsubishi_msy"),F("samsung_aqv"),F("samsung_fjm"),F("sharp"),F("mitsubishi_heavy_zj"),F("mitsubishi_heavy_zm"),F("hyundai"),F("hisense_aud"),F("gree"),F("fuego"),F("ballu")};
  opt=F(" <option value='-1' "); 
  if(mdl=="") opt+=F("selected "); 
  opt+=F("> </option>");

 for(i=0;i<23;i++)
 {
  opt+=F("<option value='");
  opt+=tab[i];
  opt+=F("' "); 
  if(mdl==tab[i]) opt+=F("selected "); 
  opt+=F(">");
  opt+=tab[i];
  opt+=F("</option>");
  }
  
}

void Web::gpio(String &opt , uint8_t gpio)
{
  opt=F(" <option value='-1' "); 
  if(gpio==-1) opt+=F("selected "); 
  opt+=F(">Off</option>");
  opt+=F("<option value='0' ");
  if(gpio==0) opt+=F("selected ");
  opt+=F(">GPIO-0 (D3)</option><option value='1' ");
  if(gpio==1) opt+=F("selected ");
  opt+=F(">GPIO-1 (D10)</option><option value='2' ");
  if(gpio==2) opt+=F("selected ");
  opt+=F(">GPIO-2 (D4)</option><option value='3' ");
  if(gpio==3) opt+=F("selected ");
  opt+=F(">GPIO-3 (D9)</option><option value='4' ");
  if(gpio==4) opt+=F("selected ");
  opt+=F(">GPIO-4 (D2)</option><option value='5' ");
  if(gpio==5) opt+=F("selected ");
  opt+=F(">GPIO-5 (D1)</option><option value='9' ");
  if(gpio==9) opt+=F("selected ");
  opt+=F(">GPIO-9 (D11)</option><option value='10' ");
  if(gpio==10) opt+=F("selected ");
  opt+=F(">GPIO-10 (D12)</option><option value='12' ");
  if(gpio==12) opt+=F("selected ");
  opt+=F(">GPIO-12 (D6)</option><option value='13' ");
  if(gpio==13) opt+=F("selected ");
  opt+=F(">GPIO-13 (D7)</option><option value='14' ");
  if(gpio==14) opt+=F("selected ");
  opt+=F(">GPIO-14 (D5)</option><option value='15' ");
  if(gpio==15) opt+=F("selected ");
  opt+=F(">GPIO-15 (D8)</option><option value='16' ");
  if(gpio==16) opt+=F("selected ");
  opt+=F(">GPIO-16 (D0)</option>");
}

void Web::formconfig(Application *c, Thermostat *t, String &str,String &error)
{
if (! hasAccess(c,str)) return;
head(str);
  if (error!="") {
    str+=F("<div class=\"msg\"><strong>");
    str+=error;
    str+=F("</strong></div>");
  }
  Add_form(str, F("config"));
  Add_table(str, F("Server configuration"));
  Add_input(str,F("spwd"),F("Password"),c->pwd);
  str+=F("</tr></tbody></table>");
  Add_table(str, F("Network configuration"));
  Add_input(str,F("ssid"),F("Ssid"),c->Wssid);
  str+=F("</tr><tr>");
  Add_input(str,F("pwd"),F("Password"),c->Wpass);
  str+=F("</tr><tr>");
  Add_input(str,F("ip"),F("IP Adress"),c->Ip);
  str+=F("</tr><tr>");
  Add_input(str,F("gat"),F("IP Gateway"),c->Gat);
  str+=F("</tr><tr>");
  Add_input(str,F("dns"),F("IP DNS"),c->Dns);  
  str+=F("</tr><tr>");
  Add_input(str,F("ntp"),F("NTP server"),c->Ntp); 
  str+=F("</tr><tr>");
  Add_input(str,F("tzone"),F("Time Zone"),String(c->Timezone)); 
  str+=F("</tr><tr>");
  str+=F("</tbody></table>");
  Add_table(str, F("Sensor configuration"));
  str+=F("<td><label for=\"dhtpin\">DHT Pin</label></td><td><select name=\"dhtpin\" id=\"dhtpin\">");
  String opt1="";
  gpio(opt1,c->dhtPin);
  str+=opt1;
  str+=F("</select></td></tr>");
  str+=F("<tr><td><label for=\"dhttype\">DHT Type></label></td><td><select name=\"dhttype\" id=\"dhttype\"><option value='-1'></option><option value='DHT11'>DHT11</option><option value='DHT22' selected>DHT22</option></select></td></tr>");
  str+=F("<tr><td><label for=\"pirpin\">PIR Pin</label></td><td><select name=\"pirpin\" id=\"pirpin\">");
  String opt2="";
  gpio(opt2,c->pirPin);
  str+=opt2;
  str+=F("</select></td></tr>");
  str+=F("<tr><td><label for=\"irpin\">IR Pin</label></td><td><select name=\"irpin\" id=\"irpin\">");
  String opt3="";
  gpio(opt3,c->irPin);
  str+=opt3;
  str+=F("</select></td></tr>"); 
  Add_input(str,F("HumE"),F("Calibrate Humidity"),String(c->HumE));
  str+=F("</tr><tr>");
  Add_input(str,F("TempE"),F("Calibrate Temperature"),String(c->TempE));
  str+=F("</tr><tr>");
  str+=F("</tbody></table>"); 
  Add_table(str, F("HeatPump configuration"));
  str+=F("<td><label for=\"model\">HeatPump model</label></td><td><select name=\"model\" id=\"model\">");
  String opt4="";
  model(opt4,c->AP);
  str+=opt4;
  str+=F("</select></td></tr></tbody></table>");
  Add_table(str, F("Thermostat configuration"));
  Add_input(str,F("ecoT"),F("Eco temperature"),String(t->Eco));
  str+=F("</tr><tr>");
  Add_input(str,F("frostT"),F("FrostFree temperature"),String(t->Frostfree));
  str+=F("</tr><tr>");
  Add_input(str,F("ComfortT"),F("Comfort temperature"),String(t->Comfort));
  str+=F("</tr><tr>");
  Add_input(str,F("hysteresisT"),F("Hysteresis"),String(t->hysteresis));
  str+=F("</tr><tr>");
  Add_input(str,F("triggerHeatT"),F("threshold value for starting/stopping heat pump"),String(t->triggerHeat));
  str+=F("</tr><tr>");
  Add_input(str,F("prestime"),F("time during which user is present after detecttion in minutes"),String(t->prestime));
  str+=F("</tr><tr>");
  str+=F("<td><label for=\"pir\">if Eco mode &PIR detection ->Comfort mode</label></td><td><input type=\"checkbox\" name=\"presentbool\" id=\"presentbool\" value=\"1\"");
  if (t->presentbool== 1) str+=F(" checked ");
     str+=F("/></td>");
str+=F("<td><label for=\"presmin\">between</label></td><td><input type=\"number\" name=\"presmin\" style=\"width: 2em;\" id=\"presmin\" min=\"0\" max=\"24\" step=\"1\" value=\"");
str+=String(t->presmin);
str+=F("\"/></td>");
str+=F("<td><label for=\"presmax\">and</label></td><td><input type=\"number\" name=\"presmax\" id=\"presmax\" style=\"width: 2em;\"  min=\"0\" max=\"24\" step=\"1\" value=\"");
str+=String(t->presmax);
str+=F("\"/></td></tr>");
str+=F("</tbody></table>");
Add_button(str, F("btn-b"),F("Submit"));
str+=F("</form></br>");
Add_hrefbutton(str,F("btn-a"),F("Thermostat Planning configuration"),F("planning"));
Add_hrefbutton(str,F("btn-a"),F("Domoticz configuration"),F("Dombroadcaster"));
if (ESP.getFlashChipRealSize() > 524288)
  {
  Add_hrefbutton(str,F("btn-a"),F("Firmware update"),F("up_firmware"));
  }
  str+=F("</br>");
Add_hrefbutton(str,F("btn-a"),F("Save Config"),F("saveconfigfile"));
  str+=F("<form method=\"post\" enctype=\"multipart/form-data\" action=\"/loadconfigfile\"><input type=\"file\" name=\"upload\" id=\"fileUpload\">");
  Add_button(str, F("btn-a"),F("Upload config"));
str+=F("</form>");
end(str);
}


//planning form
void Web::formplanning(Thermostat *t,String &str,String &error)
{
  uint8_t i=0;
 /* TODO
  *  if (c.Access== false && c.pwd != "") {String error="Password required";
 formlogging(str,error);
 return;
}
if (! hasAccess(c,str)) return;*/
head(str);
if (error!="") {
        str+=F("<div class=\"msg\"><strong>");
        str+=error;
        str+=F("</strong></div>");
    }
 Add_hrefbutton(str,F("btn-c"),F("Return"),F("config"));  
 //Add_table(str, F("Thermostat Planning for the Auto mode"));
 str+=F("<h2>");
 str+=F("Thermostat Planning for the Auto mode");
 str+=F("</h2><table class=\"table\"><thead><tr><th>Active / Inactive</th><th>Time</th><th>Mode</th><th>Day</th><th> </th</tr></thead><tbody><tr>");
    if(t->getCal() !=NULL){
    for (i=0; i<t->getnbCal();i++)
    {
    str+=F("<tr><td>");
    //str+=t->getCal()[i];


uint8_t commaIndex = t->getCal()[i].indexOf(',');
    String firstValue = t->getCal()[i].substring(0, commaIndex);
    if (firstValue == "1") 
      str+=F("A");
      else str+=F("I");
      uint8_t hourindex = t->getCal()[i].indexOf(':', commaIndex + 1); 
      uint8_t secondcommaIndex = t->getCal()[i].indexOf(',', commaIndex + 1);
      uint8_t thirdcommaIndex = t->getCal()[i].indexOf(',', secondcommaIndex + 1);
      str+=F("</td><td>");
      str+=t->getCal()[i].substring(commaIndex + 1, hourindex);
      str+=F(":");
      str+=t->getCal()[i].substring(hourindex + 1,secondcommaIndex);
      str+=F("</td><td>");
      str+=t->getCal()[i].substring(secondcommaIndex + 1, thirdcommaIndex); 
      str+=F("</td><td>");
      String fourthValue = t->getCal()[i].substring(thirdcommaIndex + 1); // To the end of the string
      switch(fourthValue.charAt(0))
        {
          case '1':
          str+=F("Sunday");
          break;

          case '2':
          str+=F("Monday");
          break;

          case '3':
          str+=F("Tuesday");
          break;

          case '4':
          str+=F("Wednesday");
          break;
          
          case '5':
          str+=F("Thursday");
          break;

          case '6':
          str+=F("Friday");
          break;

          case '7':
          str+=F("Saturday");
          break;
          
           case '8':
          str+=F("Week Days");
          break;
           case '9':
          str+=F("Week-end");
          break;
          case 'A':
          str+=F("Every day");
          break;
          default:
          str+=fourthValue;
        }
    
    //Add_hrefbutton(str,F("btn-c"),F("Del"),"delplanning?p="+String(i)); 
    str+=F("</td><td><a href=\"/delplanning?p=");
    str+=String(i);
    str+=F("\" class=\"btn btn-sm btn-c\">Del</a></td></tr>"); 
    }}
    str+=F("</tbody></table>");
Add_form(str, F("addplanning"));
Add_table(str, F("Add Planning"));
str+=F("<td><label for=\"enabled\">Enabled </label></td><td><input id=\"enabled\" name=\"enabled\" checked type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"Thour\">Time </label></td><td><input type=\"Thour\" name=\"Thour\" style=\"width: 2em;\" id=\"Thour\" min=\"0\" max=\"24\" step=\"1\" value=\"0\">");
str+=F("::<input type=\"Tmin\" name=\"Tmin\" style=\"width: 2em;\" id=\"Tmin\" min=\"0\" max=\"59\" step=\"1\" value=\"0\"></td></tr>");
str+=F("<tr><td><label for=\"command\">Command </label></td>");
str+=F("<td><select id=\"command\" name=\"command\"><option value=\"Off\" selected=\"selected\">Off</option>");
str+=F("<option value=\"Frostfree\">Frostfree</option>");
str+=F("<option value=\"Eco\">Eco</option>");
str+=F("<option value=\"Comfort\">Comfort</option>");
str+=F("<option value=\"Forced\">Forced</option>");
str+=F("</select></td></tr>");
str+=F("<tr><td><label for=\"when1\">Everyday</label></td>");
str+=F("<td><input name=\"when1\" id=\"when1\" value=\"A\" type=\"radio\"></td></tr>");
str+=F("<tr><td><label for=\"when2\">Weekdays</label></td>");
str+=F("<td><input name=\"when2\" id=\"when2\" value=\"8\" type=\"radio\"></td></tr>");
str+=F("<tr><td><label for=\"when3\">Weekends</label></td>");
str+=F("<td><input name=\"when3\" id=\"when3\" value=\"9\" type=\"radio\"></td></tr>");
str+=F("<tr><td><label for=\"when4\">Selected Days</label></td>");
str+=F("<td><input name=\"when4\" id=\"when4\" value=\"day\" type=\"radio\" checked></td></tr>");
str+=F("<tr><td></td><td>");
Add_table(str, F("Days"));
str+=F("<tr><td><label for=\"ChkMon\">Monday</label></td>");
str+=F("<td><input name=\"ChkMon\" id=\"ChkMon\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkTue\">Tuesday</label></td>");
str+=F("<td><input name=\"ChkTue\" id=\"ChkTue\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkWed\">Wednesday</label></td>");
str+=F("<td><input name=\"ChkWed\" id=\"ChkWed\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkThu\">Thursday</label></td>");
str+=F("<td><input name=\"ChkThu\" id=\"ChkThu\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkFri\">Friday</label></td>");
str+=F("<td><input name=\"ChkFri\" id=\"ChkFri\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkSat\">Satursday</label></td>");
str+=F("<td><input name=\"ChkSat\" id=\"ChkSat\" type=\"checkbox\" value=\"1\"></td></tr>");
str+=F("<tr><td><label for=\"ChkSun\">Sunday</label></td>");
str+=F("<td><input name=\"ChkSun\" id=\"ChkSun\" type=\"checkbox\" value=\"1\"></td>");
str+=F("</tr></tbody></table>");
str+=F("</td></tr></tbody></table>");
Add_button(str, F("btn-b"),F("Submit"));
str+=F("</form></div>");
end(str);
}

//log form
void Web::formlog(Application *lc,String &str)
{
if (! hasAccess(lc,str)) return;

uint8_t i=0;
uint8_t nlogs=lc->getNblogs();

head(str);
Add_refresh(str);
Add_table(str, F("Logs"));
if (lc->getNblogs() ==0) str+=F("No logs</td></tr>");
else{
  i=lc->getFirstlog();
  do {
      str+=F("<tr><td>");
      str+=lc->getLogs()[i];
      str+=F("</td></tr>");
      i=(i+1)% NBLOGS;
      nlogs--;
      }
  while(nlogs>0);
  }
str+=F("</tbody></table>");
str+=F("<p>Free Memory = ");
str+= ESP.getFreeHeap();  
str+=F("</p></div>");
end(str);       
}

//logging form
void Web::formlogging(String &str,String &error)
{
forehead(str);
str+=F("<div class=\"container\"><div class=\"hero\">");
if (error!="") {
        str+=F("<div class=\"msg\"><strong>");
        str+=error;
        str+=F("</strong></div></p>");
    }
    Add_form(str, F(""));
    Add_table(str, F("Logging"));
    Add_input(str,F("pwd"),F("Password "),"");
    str+=F("</tr></tbody></table>");
    Add_button(str, F("btn-b"),F("Submit"));
    str+=F("</form></div>");
    end(str);
}

//Domoticz form
void Web::formdomo(Application *lc,String &str, String &error)
{
if (! hasAccess(lc,str)) return;
head(str);
 Add_hrefbutton(str,F("btn-c"),F("Return"),F("config"));  
if (error!="") {
        str+=F("<div class=\"msg\"><strong>");
        str+=error;
        str+=F("</strong></div>");
    }
    Add_form(str, F("savedomo"));
    Add_table(str, F("Domoticz config"));
str+=F("<td><label for=\"domo\">Domoticz Use</label></td><td><input type=\"checkbox\" name=\"domo\" id=\"domo\" value=\"1\"");
if (lc->domo== true) str+=F(" checked ");
str+=F("\"/></td></tr>");
Add_input(str,F("domoIP"),F("Domoticz server IP"),String(lc->domoIP));
str+=F("</tr><tr>");
Add_input(str,F("domoPir"),F("Domoticz PIR idx"),String(lc->domoPir));
str+=F("</tr><tr>");
Add_input(str,F("domoTemp"),F("Domoticz Temp idx"),String(lc->domoTemp));
str+=F("</tr><tr>");
Add_input(str,F("domogTemp"),F("Domoticz get Temp idx"),String(lc->domogTemp));
str+=F("</tr><tr>");
Add_input(str,F("domoMode"),F("Domoticz selector idx (Selector with labels Off, Auto, Frostfree, Eco, Comfort, Forced)"),String(lc->domoMode));
str+=F("</tr></tbody></table>");
Add_button(str, F("btn-b"),F("Submit"));
str+=F("</form></div>");
end(str);
}

