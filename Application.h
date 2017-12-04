#ifndef Application_h
#define Application_h

//#define DEBUG 
#ifdef DEBUG
  #define DEBUG_PRINT(x)     Serial.print(x)
  #define DEBUG_PRINTDEC(x)     Serial.print(x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x) 
#endif

#define NBLOGS 15

#include <Time.h>
#include <WString.h>
#include <FS.h>

class Application
{
private:
String file;
String * Logs=NULL;
uint8_t nblogs=0;
uint8_t firstlog=0;

public:
    float TempE;
    float HumE;
    String Wssid;
    String Wpass;
    uint8_t  dhtType;
    uint8_t dhtPin;
    uint8_t pirPin;
    uint8_t irPin;
    String AP;
    uint8_t Cmd; 
    String Ntp;
    uint8_t Timezone;
    String Ip;
    String Gat;
    String Dns;
    String Mode;
    float Temp;
    float FTemp;
    String pwd;
    bool Access=false;
    bool domo;
    String domoIP;
    unsigned int domoPir;
    unsigned int domoTemp;
    unsigned int domoMode;
    unsigned int domogTemp;
    
Application();
Application(String s);
void save ();
void load();
void init();
void AddLog(String);
uint8_t getNblogs() {return nblogs;};
uint8_t getFirstlog() {return firstlog;};
String * getLogs() {return Logs;};
String getfile();
};

#endif


