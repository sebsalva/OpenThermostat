#ifndef WifiManager_h
#define WifiManager_h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Application.h"

class WifiManager
{
private:
    Application *lconfig;

    void initAP();
    bool connexion(char *ssid, char *pwd, int expiration);
    String ipToString(IPAddress ip);
    void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
public:
WifiManager(Application *c);
void initWifi();
String getWiFiInfo();
void disconnect();
};

#endif

