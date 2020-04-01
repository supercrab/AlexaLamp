#include <EEPROM.h>
#include "Config.h"

// Load config settings from EEPROM
bool Config::load(){
	// Initialise EEPROM
	EEPROM.begin(512);
	// check for previous settings 
	if (EEPROM.read(0) == 'L' && EEPROM.read(1) == 'A' && EEPROM.read(2) == 'M' && EEPROM.read(3) == 'P'){
		_wifi = EEPROM.read(4);
		Serial.printf("Lamp settings: found in EEPORM. Wifi enabled: %s\n", _wifi ? "YES" : "NO" );
		return true;
	}
	else{
		Serial.println("Lamp settings: none found in EEPROM. Using defaults!");
		return false;
	}
}

// Store our config settings
bool Config::save(){
	// Write our special string
	EEPROM.write(0, 'L');    
	EEPROM.write(1, 'A');   
	EEPROM.write(2, 'M');
	EEPROM.write(3, 'P');
	EEPROM.write(4, _wifi);
	// Save changes
	if (EEPROM.commit()){
		Serial.printf("Lamp settings: saved wifi enabled: %s\n", _wifi ? "YES" : "NO" );
		return true;
	}
	else{
		Serial.println("Lamp settins: not saved!");
		return false;
	}
}

// Is the lamp wifi enabled?
bool Config::getWifi(){
	return _wifi;
}
void Config::setWifi(bool enabled){
	_wifi = enabled;
}
void Config::invertWifi(){
	_wifi = !_wifi;
}

// Is the lamp on or off?
bool Config::getState(){
	return _state;
}
void Config::setState(bool state){
	_state = state;
}
void Config::invertState(){
	_state = !_state;
}

// How bright is the lamp
uint8_t Config::getBrightness(){
	return _brightness;
}
void Config::setBrightness(uint8_t brightness){
	_brightness = brightness;
}

// Rotary encoder sensitivity
uint8_t Config::getEncoderSensitivity(){
	return _encoderSensitivity;
}
void Config::setEncoderSensitivity(uint8_t sensitivity){
	_encoderSensitivity = sensitivity;
}

// How many minutes to check wifi connection
uint8_t Config::getWifiCheckInterval(){
	return _wifiCheckInterval;
}
void Config::setWifiCheckInterval(uint8_t interval){
	_wifiCheckInterval = interval;
}