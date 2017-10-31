#ifndef IRManager_h
#define IRManager_h

#include <FujitsuHeatpumpIR.h>
#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>
#include <CarrierHeatpumpIR.h>
#include <MideaHeatpumpIR.h>
#include <MitsubishiHeatpumpIR.h>
#include <SamsungHeatpumpIR.h>
#include <SharpHeatpumpIR.h>
#include <DaikinHeatpumpIR.h>
#include <DaikinHeatpumpARC417IR.h>
#include <MitsubishiHeavyHeatpumpIR.h>
#include <HyundaiHeatpumpIR.h>
#include <HisenseHeatpumpIR.h>
#include <GreeHeatpumpIR.h>
#include <FuegoHeatpumpIR.h>
#include <ToshibaHeatpumpIR.h>
#include <BalluHeatpumpIR.h>
#include "Application.h"

class IRManager
{
private:
    IRSender *irSender;
    Application *lconfig;
    unsigned int HPState=0;
float HPTemp=0;
public:
uint8_t  getHPState() {return HPState;};
uint8_t getHPTemp() {return HPTemp;};
IRManager();
IRManager(Application *c);
uint8_t sendIR(String hpmodel, String power, String fmode, String fan, String temp, String vair, String hair);
void init();
};

#endif

