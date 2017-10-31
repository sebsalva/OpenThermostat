#ifndef TimeNtp_h
#define TimeNtp_h

#include <TimeLib.h>
#include <ESP8266WiFi.h> 
#include <WiFiUdp.h> 

#include "Application.h"

class TimeNtp
{
private:
static TimeNtp* getTimeObject;
WiFiUDP Udp; 
// default NTP Servers: 
String ntpServerName="fr.pool.ntp.org";
uint8_t timeZone = 2;     // paris france
unsigned int localPort = 8888;  // local port to listen for UDP packets 
static const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message 
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets 
time_t getNtpTime();
void sendNTPpacket(IPAddress &address); 
static time_t globalGetNTPTime();
bool summertime(int year, byte month, byte day, byte hour, byte tzHours);
unsigned int tempo=20;

public:
bool gotTime=false;
TimeNtp();
TimeNtp(String server, int zone);
time_t getTime(void);
void init(String server,int zone);
};

#endif

