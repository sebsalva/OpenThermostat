#include "IRManager.h"

IRManager::IRManager()
{
irSender=new IRSenderBitBang(lconfig->irPin);
}

IRManager::IRManager(Application *c)
{
  lconfig=c;
}

void IRManager::init()
{
irSender = new IRSenderBitBang(lconfig->irPin);
  
}


// Quand l'utilisateur fais appel au service /sendIR
//0 -> error
//1-> ok
uint8_t IRManager::sendIR(String hpmodel, String power, String fmode, String fan, String temp, String vair, String hair)
{
unsigned int result = 0;
  
 DEBUG_PRINTLN(F("IR called"));


// Liste des objets permettant d'envoyer une signal IR par modÃ¨le de pompe Ã  chaleur
HeatpumpIR *heatpumpIR[] = {new PanasonicCKPHeatpumpIR(), new PanasonicDKEHeatpumpIR(), new PanasonicJKEHeatpumpIR(),
             new PanasonicNKEHeatpumpIR(), new CarrierNQVHeatpumpIR(), new CarrierMCAHeatpumpIR(),
             new MideaHeatpumpIR(), new FujitsuHeatpumpIR(),
             new MitsubishiFDHeatpumpIR(), new MitsubishiFEHeatpumpIR(), new MitsubishiMSYHeatpumpIR(),
             new SamsungAQVHeatpumpIR(), new SamsungFJMHeatpumpIR(), new SharpHeatpumpIR(), new DaikinHeatpumpARC417IR(), new DaikinHeatpumpIR(),
             new MitsubishiHeavyZJHeatpumpIR(), new MitsubishiHeavyZMHeatpumpIR(),
             new HyundaiHeatpumpIR(), new HisenseHeatpumpIR(), new GreeGenericHeatpumpIR(),
             new FuegoHeatpumpIR(), new ToshibaHeatpumpIR(), new BalluHeatpumpIR(),
             NULL
};
unsigned int panasonicCKPTimer = 0; // Timer pour IR Panasonic CKP
String heatpumpModel; // Modele de l'air conditionnÃ©
  unsigned int powerMode = POWER_ON; // AllumÃ© ou Ã©teint
  unsigned int operatingMode = MODE_HEAT; // Mode de fonctionnement de l'air conditionnÃ©
  unsigned int fanSpeed = FAN_2; // Vitesse du ventialteur
  unsigned int temperature = 22; // TempÃ©rature souhaitÃ©e
  unsigned int vDir = VDIR_UP; // Direction verticale de la soufflerie
  unsigned int hDir = HDIR_AUTO; // Direction horizontale de la soufflerie

  // Attribution des valeurs saisies par l'utilisateur
  heatpumpModel = hpmodel;
  powerMode = power.toInt();
  operatingMode = fmode.toInt();
  fanSpeed = fan.toInt();
  temperature = temp.toInt();
  vDir = vair.toInt();
  hDir = hair.toInt();

  // Si l'utilisateur a saisit des valeurs valides
  if (powerMode >= 0 && powerMode <= 1 && operatingMode >= 0 && operatingMode <= 6 && fanSpeed >= 0 && fanSpeed <= 5 && vDir >= 0 && vDir <= 6 && hDir >= 0 && hDir <= 6)
  {
    DEBUG_PRINTLN(F("IR sending"));
    int i = 0;
    // Parcours de la liste des modÃ¨les d'air conditionnÃ© supportÃ©s par la librairie
    do
    {
      // RÃ©cupÃ©ration du nom de l'objet de l'air condtionnÃ© en cours
      const char* shortName = heatpumpIR[i]->model();
      const char* longName = heatpumpIR[i]->info();

      // Si les noms de l'air conditionnÃ© correspodent alors on envoie un signal IR
      if (strcmp_P(heatpumpModel.c_str(), shortName) == 0)
      {
        // Envoi du signal IR avec les donnÃ©es saisies par l'utilisateur
        heatpumpIR[i]->send(*irSender, powerMode, operatingMode, fanSpeed, temperature, vDir, hDir);
        result = 1;
        DEBUG_PRINTLN(F("IR send Done"));
        this->HPState= powerMode;
        this->HPTemp=(float)temperature;
        // Panasonic CKP can only be turned ON/OFF by using the timer,
        // so cancel the timer in 2 minutes, after the heatpump has turned on or off
        if (strcmp(heatpumpModel.c_str(), "panasonic_ckp") == 0)
        {
          panasonicCKPTimer = 120;
        }
        break;
      }
    }
    while (heatpumpIR[++i] != NULL);
  }
  delay(100);
  return result;
}

