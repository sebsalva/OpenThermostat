#include "WifiManager.h"

WifiManager::WifiManager(Application *c)
{
lconfig=c;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// MÃ©thodes de connexion Wifi
// ------------------------------------------------------------------------------------------------------------------------------------------------------

void WifiManager::disconnect()
{
WiFi.disconnect();
}

// Initialisation du point d'accÃ¨s
void WifiManager::initAP()
{
  WiFi.mode(WIFI_AP); //WIFI_AP_STA); // Mode point d'accÃ¨s
  IPAddress ip(192, 168, 1, 1);
  IPAddress gateway(1, 2, 3, 1);
  IPAddress subnet(255, 255, 255, 0);  
  WiFi.softAPConfig(ip, gateway, subnet);
  IPAddress apip = WiFi.softAPIP();
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 " + macID;
  const char WiFiAPPSK[] = "password";

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
  DEBUG_PRINTLN(F("Wifi AP mode"));
  lconfig->AddLog(F("Wifi AP mode"));  
}


// Initialisation du Wifi
void WifiManager::initWifi()
{
  // Si le ssid n'est pas ESP8266 (le fichier de sauvegarde contient des donnÃ©es de connexions sauvegardÃ©es)
  if (lconfig->Wssid == "ESP8266")
  {
    initAP(); // Initialisation du point d'accÃ¨s
  }
  else // Sinon le fichier de sauvegarde n'a pas de donnÃ©es de connexion sauvegardÃ©es
  {
     // Conversion String en char*
    char* ssid = new char[lconfig->Wssid.length()+1];
    strcpy(ssid, lconfig->Wssid.c_str());
    // Conversion String en char*
    char* pwd = new char[lconfig->Wpass.length()+1];
    strcpy(pwd, lconfig->Wpass.c_str()); 
    WiFi.mode(WIFI_STA);
 WiFi.disconnect(); // Fermeture d'une eventuelle connexion
   
    digitalWrite(LED_BUILTIN, HIGH); // Eteindre la LED

    // Si la connexion a fonctionnÃ© alors on sauvegarde les donnÃ©es de connexion dans la mÃ©moire
    if (connexion(ssid, pwd, 20)) // 20 = 20 secondes avant l'estimation de l'echec
    {
      //saveWifiCredentials(ssid, pwd);
      lconfig->AddLog("Connected to network");
    }
    else // Sinon on lance le point d'accÃ¨s WiFi pour que l'utilisateur se connecte et modifie les donnÃ©es de connexion
    {
      WiFi.disconnect(); 
      initAP();
    }
  }
}

void WifiManager::parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

// Connexion au Wifi : retourne true si reussi, sinon false
bool WifiManager::connexion(char *ssid, char *pwd, int expiration)
{
  bool aRetourner = false;
  // IP config test
  DEBUG_PRINT(F("Chosen IP: "));
  DEBUG_PRINTLN(lconfig->Ip);

  //lconfig->AddLog("IP: "+lconfig->Ip);
  if (lconfig->Ip!="0.0.0.0" && lconfig->Ip!="")
    {
  byte ip[4];
  parseBytes(lconfig->Ip.c_str(), '.', ip, 4, 10);
  byte gat[4];
  parseBytes(lconfig->Gat.c_str(), '.', gat, 4, 10);
  byte dns[4];
  parseBytes(lconfig->Dns.c_str(), '.', dns, 4, 10);
  WiFi.config(ip, dns, gat); 
  }
  WiFi.begin(ssid, pwd); // Initialisation de la connexion
  DEBUG_PRINT(F("Working to connect"));
  
  int timeout = 0; // Temps d'attente de la connexion avant echec
  // Attendre la connexion, si ellsee prends plus de 10 secondes, impossible de se connecter
  while (WiFi.status() != WL_CONNECTED && timeout < expiration) {
    // Clignotement de la LED pour notifier la volontÃ© de connexion et incrÃ©mentation du timer d'expiration
    digitalWrite(LED_BUILTIN, LOW);
    timeout++;
    delay(500);
    DEBUG_PRINT(F("."));
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  if (timeout < expiration) // Si la connexion a reussi
  {
    digitalWrite(LED_BUILTIN, LOW); // Allumer la LED pour notifier que la carte est connectÃ©e
    DEBUG_PRINT(F("IP address: "));
    DEBUG_PRINTLN(WiFi.localIP());
    lconfig->AddLog("IP address: "+WiFi.localIP());
    aRetourner = true;
     }
  else // Si la connexion a Ã©chouÃ©
  {
    digitalWrite(LED_BUILTIN, HIGH); // Eteindre la LED pour notifier l'echec de connexion
  }
  return aRetourner;
}

// Retourne les informations de connexion de la carte
String WifiManager::getWiFiInfo()
{
 String adresseIP = ipToString(WiFi.localIP());
 String aRetourner = WiFi.SSID() + ";" + adresseIP + ";" + lconfig->Wssid + ";" + lconfig->Wpass + ";";
 // String aRetourner = lconfig->Wssid + ";" + lconfig->Wpass + ";";
 
  return aRetourner;
}

// Convertit un objet adresse ip en chaÃ®ne de caractÃ¨res affichable
String WifiManager::ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

