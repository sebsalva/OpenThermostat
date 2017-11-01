# OpenThermostat
Thermostat for Heatpump using ESP 8266

### Features
* Several modes: Frost, Eco, Comfort, Forced (choose your temperature with thermostat) and Auto
* Mode Auto uses Calendar
* Stop Heatpump if temperature over a given threshold
* Start Heatpump if temperature below a given threshold
* Uses PIR : if detection of someone (at least 3 times during 3min) and mode Eco selected -> triggers the Mode comfort for 30 minutes. Stay in Mode Confort if continuous detections, or go back to Mode Eco after 30 minutes.
* Connects to Ntp server to get correct date and time
* Setup through a Web interface
* Firmware update OTA
* Broadcast info (mode, temperature, PIR) to Domoticz

### Prerequisites
```ESP8266 with the folowwing devices: PIR(optional), DHT22 (temperature and humidity, optional), IR (with HeatPumpIR plugin)```
![Hardware](https://raw.githubusercontent.com/sasa27/Domoticz-Heatpump-Thermostat/master/hardware.png)

### Installing
```
Compile with ArduinoIDE and flash your ESP card
First connection on a Wifi network SSID "ESP8266" followed by the last digit of the card MAC adress
Initial passwords (Wifi+App): esppwd
```
![Openth Main](https://raw.githubusercontent.com/sasa27/OpenThermostat/master/openth-main.png)
![Openth Config](https://raw.githubusercontent.com/sasa27/OpenThermostat/master/openth-config11.png)
![Openth Config](https://raw.githubusercontent.com/sasa27/OpenThermostat/master/openth-config2.png)
![Openth config Domoticz](https://raw.githubusercontent.com/sasa27/OpenThermostat/master/openth-domoticz.png)
![Openth Log](https://raw.githubusercontent.com/sasa27/OpenThermostat/master/openth-log.png)
