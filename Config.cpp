#include "Config.h"

// Address of user settings in EEPROM
// (the first 4 bytes are used to identify this system)
#define SETTINGS_ADDRESS 8

// Pass in functions needed to carry out actions when light is updated or settings are updated
void Config::begin(){

	// Setup our default settings
	strncpy(_bag.deviceName, DEFAULT_ALEXA_NAME, ALEXA_NAME_LENGTH);

	_bag.mode = (uint8_t) DEFAULT_MODE;
	_bag.encoderInverted = DEFAULT_ENCODER_INVERTED;
	_bag.encoderSensitivity = DEFAULT_ENCODER_SENSITIVITY;
	_bag.wifiCheckInterval = DEFAULT_WIFI_CONNECTED_CHECK_MINS;
	_bag.automaticUpdates = DEFAULT_AUTOMATIC_UPDATES;

	// Default update status
	setUpdateStatusIdle();

	// Load previous settings are they are available
	load();
}

// Assign light state/brightness change callback 
void Config::onLightChanged(void(*lightCallback)(void)){
	_updateLightCallback = lightCallback;
}

// Assign settings change callback
void Config::onSettingsChanged(void(*settingsCallback)(void)){
	_updateSettingsCallback = settingsCallback;
}

// Assign encoder inverted callback
void Config::onEncoderInverted(void(*invertEncoderCallback)(void)){
	_invertEncoderCallback = invertEncoderCallback;
}

// Load config settings from EEPROM
bool Config::load(){
	bool ret;

	// Initialise EEPROM
	EEPROM.begin(512);

	// Check for magic bytes which denotes settings are available 
	if (
		EEPROM.read(0) == 'L' &&
		EEPROM.read(1) == 'A' &&
		EEPROM.read(2) == 'M' &&
		EEPROM.read(3) == 'P' &&
		EEPROM.read(4) == '*'
	){
		EEPROM.get(SETTINGS_ADDRESS, _bag);
		Serial.println(F("SETTINGS: available in EEPORM"));
		ret = true;
	}
	else{
		Serial.println(F("SETTINGS: not found in EEPROM - using defaults!"));
		ret = false;
	}

	// Release resources
	EEPROM.end();

	// Display current settings
	_outputSettings();

	// Do we need to retain previous state after the system has been updated?
	if (ret){
		_initialisePreviousState();
	}

	// Did the read work?
	return ret;
}

// Do we need to initialise previous state after a system update?
void Config::_initialisePreviousState(){
	if (_bag.initialState){
		Serial.println(F("Previous state initialised!"));
		Serial.print("Brightness: "); Serial.println(_bag.initialBrightness);

		// Set previous state and brightness 
		_state = true;
		_brightness = _bag.initialBrightness;

		// Clear this previous state so the state & brightness so that it is not retained on next restart
		_bag.initialState = false;
		_bag.initialBrightness = 0;

		// Save the setting
		save();
	}
}

// Store our config settings
bool Config::save(){
	bool ret;

	// Initialise EEPROM
	EEPROM.begin(512);

	// Write our magic bytes
	EEPROM.write(0, 'L');    
	EEPROM.write(1, 'A');   
	EEPROM.write(2, 'M');
	EEPROM.write(3, 'P');
	EEPROM.write(4, '*');

	// Store our struct
	EEPROM.put(SETTINGS_ADDRESS, _bag);

	// Save changes
	ret = EEPROM.commit();
	if (ret){
		Serial.println(F("SETTINGS: saved OK"));
	}
	else{
		Serial.println(F("SETTINGS: failed to save!"));
	}

	// Release resources
	EEPROM.end();

	// Display current settings
	_outputSettings();

	// Did the save work?
	return ret;
}

// Output settings
void Config::_outputSettings(){
	Serial.println(F("+-------------------+"));
	Serial.println(F("|   User Settings   |"));
	Serial.println(F("+-------------------+"));
	Serial.print(F("Name: ")); Serial.println(_bag.deviceName);
	Serial.print(F("State: ")); Serial.println(_state ? F("on") : F("off"));
	Serial.print(F("Brightness: ")); Serial.println(_brightness);
	Serial.print(F("Mode: ")); Serial.println(_bag.mode ? F("wifi") : F("standalone"));
	Serial.print(F("Encoder inverted: ")); Serial.println(_bag.encoderInverted);
	Serial.print(F("Encoder sensitivity: ")); Serial.println(_bag.encoderSensitivity);
	Serial.print(F("Wifi check interval: ")); Serial.println(_bag.wifiCheckInterval);
	Serial.print(F("Automatic updates: ")); Serial.println(_bag.automaticUpdates);
	Serial.print(F("Initial state: ")); Serial.println(_bag.initialState);
	Serial.print(F("Initial brightness: ")); Serial.println(_bag.initialBrightness);
	Serial.println();
}

// Device name
char* Config::getDeviceName(){
	return _bag.deviceName;
}

uint8_t Config::getDeviceNameLength(){
	return sizeof(_bag.deviceName);
}

void Config::setDeviceName(char* deviceName){
	strncpy(_bag.deviceName, deviceName, sizeof(_bag.deviceName));
}

// What is the mode of the lamp
operation_t Config::getMode(){
	return (operation_t) _bag.mode;
}

void Config::setMode(operation_t mode){
	_bag.mode = (uint8_t) mode;
}

void Config::toggleMode(){
	_bag.mode = _bag.mode == 1 ? 0 : 1;
}

// Is the lamp on or off?
bool Config::getState(){
	return _state;
}

void Config::setState(bool state){
	_state = state;
}

void Config::toggleState(){
	_state = !_state;
}

// How bright is the lamp
uint8_t Config::getBrightness(){
	return _brightness;
}

void Config::setBrightness(uint8_t brightness){
	_brightness = brightness;
}

// Should the encoder be inverted?
bool Config::getEncoderInverted(){
	return _bag.encoderInverted;
}

void Config::setEncoderInverted(bool inverted){
	bool valueChanged = _bag.encoderInverted != inverted;
	_bag.encoderInverted = inverted;

	// If the encoder has been inverted then deal with this in main thread
	if (valueChanged && _invertEncoderCallback != NULL){
		_invertEncoderCallback();
	}
}

// Rotary encoder sensitivity
uint8_t Config::getEncoderSensitivity(){
	return _bag.encoderSensitivity;
}

void Config::setEncoderSensitivity(uint8_t sensitivity){
	_bag.encoderSensitivity = sensitivity;
}

// How many minutes to check wifi connection
uint8_t Config::getWifiCheckInterval(){
	return _bag.wifiCheckInterval;
}

void Config::setWifiCheckInterval(uint8_t interval){
	_bag.wifiCheckInterval = interval;
}

// Do we want automatic updates
bool Config::getAutomaticUpdates(){
	return _bag.automaticUpdates;
}

void Config::setAutomaticUpdates(bool updates){
	_bag.automaticUpdates = updates;
}

// Update status
updateStatus_t Config::getUpdateStatus(){
	return _updateStatus;
}

// Update message 
String Config::getUpdateMessage(){
	return _updateMessage;
}

// Are we updating?
bool Config::getIsUpdating(){
	return (_updateStatus == UPDATE_STATUS_LOCAL_UPDATE) || (_updateStatus == UPDATE_STATUS_REMOTE_UPDATE);
}

// Set update statuses
void Config::setUpdateStatusIdle(){
	_updateStatus = UPDATE_STATUS_IDLE;
	_updateMessage = "";
}

void Config::setUpdateStatusNoUpdates(){
	_updateStatus = UPDATE_STATUS_NO_UPDATES;
	_updateMessage = F("No update is available");
}

void Config::setUpdateStatusLocalUpdate(){
	if (!getIsUpdating()){
		_updateStatus = UPDATE_STATUS_LOCAL_UPDATE;
		_updateMessage = F("Local update started");
	}
}

void Config::setUpdateStatusRemoteUpdate(){
	if (!getIsUpdating()){
		_updateStatus = UPDATE_STATUS_REMOTE_UPDATE;
		_updateMessage = F("Remote update started");
	}
}

void Config::setUpdateStatusFailed(String message){
	_updateStatus = UPDATE_STATUS_FAILED;
	_updateMessage = message;
}

void Config::setUpdateStatusOK(){
	_updateStatus = UPDATE_STATUS_OK;
	_updateMessage = F("Updated OK");
}

// Light has been updated so we need to handle this in the main thread
void Config::updateLight(){
	if (_updateLightCallback != NULL)
		_updateLightCallback();
}

// Settings have been updated so we need to handle this in the main thread
void Config::updateSettings(){

	// Do what we need to on the main thread
	if (_updateSettingsCallback != NULL)
		_updateSettingsCallback();

	// Save settings!
	save();
}

// Should we reboot system?
bool Config::getRebootSystem(){
	return _rebootSystem;
}

void Config::setRebootSystem(){
	_rebootSystem = true;
}


// When the system is updated, store current state and brightness before rebooting   
// When the system is restarted the lamp can be put back to it's previous state
void Config::systemUpdated(){

	// We only need to store the state if light is currently on
	if (_state){

		Serial.println(F("Saving initial settings for light"));

		_bag.initialState = true;
		_bag.initialBrightness = _brightness;

		// Save settings!
		save();
	}
}