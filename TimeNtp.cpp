#include "TimeNtp.h"

TimeNtp* TimeNtp::getTimeObject;

TimeNtp::TimeNtp()
{
}

TimeNtp::TimeNtp(String server, int zone)
{
ntpServerName=server;
timeZone=zone;
TimeNtp();
}


void TimeNtp::init(String server,int zone)
{
ntpServerName=server;
timeZone=zone;
Udp.begin(localPort);
TimeNtp::getTimeObject=this;
setSyncProvider(globalGetNTPTime); 
setSyncInterval(tempo); 
setTime(getTime());
DEBUG_PRINT(F("Today: "));
DEBUG_PRINTLN(weekday());
}



time_t TimeNtp::getTime()
{
return now();
}

//wrap the following for setSyncProvider
time_t TimeNtp::globalGetNTPTime()
  {
     return TimeNtp::getTimeObject->getNtpTime();
  }


time_t TimeNtp::getNtpTime() 
{ 
  time_t temp;
IPAddress ntpServerIP; // NTP server's ip address 
while (Udp.parsePacket() > 0) ; // discard any previously received packets 
DEBUG_PRINTLN(F("Transmit NTP Request"));
 //lconfig->AddLog("Transmit NTP Request");
 // get a random server from the pool 
char charBuf[30];
ntpServerName.toCharArray(charBuf, 30); 
WiFi.hostByName(charBuf, ntpServerIP); 
 DEBUG_PRINT(ntpServerName);
  DEBUG_PRINT(F(": ")); 
   DEBUG_PRINTLN(ntpServerIP); 
 sendNTPpacket(ntpServerIP); 
 uint32_t beginWait = millis(); 
 while (millis() - beginWait < 7000) { 
 int size = Udp.parsePacket(); 
 if (size >= NTP_PACKET_SIZE) { 
  DEBUG_PRINTLN(F("Receive NTP Response"));
       Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer 
       unsigned long secsSince1900; 
       // convert four bytes starting at location 40 to a long integer 
       secsSince1900 =  (unsigned long)packetBuffer[40] << 24; 
       secsSince1900 |= (unsigned long)packetBuffer[41] << 16; 
       secsSince1900 |= (unsigned long)packetBuffer[42] << 8; 
       secsSince1900 |= (unsigned long)packetBuffer[43]; 
     tempo=3600;
     gotTime=true;
     setSyncInterval(tempo); 
     temp=secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR; 
     if (summertime(year(temp), month(temp), day(temp), hour(temp), timeZone))
temp+=SECS_PER_HOUR;
return temp;
     } 
   } 
   DEBUG_PRINTLN(F("No NTP Response :-("));
   //lconfig->AddLog("No NTP Response :-(");
   return 0; // return 0 if unable to get the time 
 } 

// send an NTP request to the time server at the given address 
void TimeNtp::sendNTPpacket(IPAddress &address) 
{ 
   // set all bytes in the buffer to 0 
   memset(packetBuffer, 0, NTP_PACKET_SIZE); 
   // Initialize values needed to form NTP request 
   // (see URL above for details on the packets) 
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode 
   packetBuffer[1] = 0;     // Stratum, or type of clock 
   packetBuffer[2] = 6;     // Polling Interval 
   packetBuffer[3] = 0xEC;  // Peer Clock Precision 
   // 8 bytes of zero for Root Delay & Root Dispersion 
   packetBuffer[12] = 49; 
   packetBuffer[13] = 0x4E; 
   packetBuffer[14] = 49; 
   packetBuffer[15] = 52; 
   // all NTP fields have been given values, now 
   // you can send a packet requesting a timestamp: 
   Udp.beginPacket(address, 123); //NTP requests are to port 123 
   Udp.write(packetBuffer, NTP_PACKET_SIZE); 
   Udp.endPacket(); 
} 
bool TimeNtp::summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
  if ((month<3) || (month>10)) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if ((month>3) && (month<10)) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day)<(1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
    return true;
  else
    return false;
}


