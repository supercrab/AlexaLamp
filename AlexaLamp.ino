//	____ _    ____ _  _ ____       _    ____ _  _ ___  
//	|__| |    |___  \/  |__|       |    |__| |\/| |__] 
//	|  | |___ |___ _/\_ |  |       |___ |  | |  | |    
//				with brightness control
//
//	by Supercrab
//	https://github.com/supercrab
//
//	Developed on a WeMos D1 mini clone, using Arduino IDE 1.8.1 with ESP8266 Core 2.7.4
//
//  This code is intended to work on cheap ESP8266 devices
//
//	Thanks to the developers of the libraries I found on GitHub!
//
// TODO
// ====
// 
// * Lamp might flickers with values < 100, this seems to be down to the newer ESP core
// * MDNS seems to be flakey especially after you setup wifi using the automatic hotspot
//   hopefully fixed in a newer version of the ESP8266 core (not related to any code here)
// * Fauxmo - when 2 devices have the same Alexa name and you browse to the device on port 80, it causes a restart
//  (might be fixed due to the MAC address not being static now)
//
// ---------------------------------------------------------
// Main configuration
// ---------------------------------------------------------
#include "Common.h"

// ---------------------------------------------------------
//  * Fauxmoesp (vintlabs)
//  https://github.com/vintlabs/fauxmoESP
//	* Fauxmoesp redirect functionality (Enrico A.)
//	https://bitbucket.org/xoseperez/fauxmoesp/issues/101/fauxmoesp-with-esp8266webserver
// ---------------------------------------------------------
#include "fauxmoESP.h"

// ---------------------------------------------------------
//	* Dimmable-Light-Arduino (Fabiano Riccardi)
//	https://github.com/fabiuz7/Dimmable-Light-Arduino
// ---------------------------------------------------------
#include <dimmable_light.h>

// ---------------------------------------------------------
//	* AsyncWiFiManager (lbussy)
//	https://github.com/lbussy/AsyncWiFiManager
// ---------------------------------------------------------
#define WM_ASYNC // Turn on Async mode
#include <AsyncWiFiManager.h>

// ---------------------------------------------------------
//	* Encoder (Paul Stoffregen)
//	https://github.com/PaulStoffregen/Encoder
// ---------------------------------------------------------
#include <Encoder.h>

// ---------------------------------------------------------
//	* JC_Button (Jack Christensen)
//	https://github.com/JChristensen/JC_Button
// ---------------------------------------------------------
#include <JC_Button.h>

// ---------------------------------------------------------
// System libraries
// ---------------------------------------------------------
#ifdef ENABLE_MDNS
	#include <ESP8266mDNS.h>
#endif
#include <ESP8266HTTPClient.h>
#include "coredecls.h"

// ---------------------------------------------------------
// Project libraries
// ---------------------------------------------------------
#include "SoftwareTimer.h"
#include "Config.h"
#include "AdminWebServer.h"
#include "HttpUpdate.h"

fauxmoESP fauxmo;
DimmableLight light(TRIAC_PIN);
Encoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
Button button(ENCODER_SWITCH_PIN);
SoftwareTimer timerWifiCheck;
SoftwareTimer timerAutomaticUpdate;
AdminWebServer server(ADMIN_WEB_PORT);
Config config;

// Calculate PWM constant
const float pwm = (255 * log10(2))/log10(255);

// Previous encoder value
int16_t encoderOldValue = 0;

// Initialise serial port and communications
void setupSerial(){
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println();
	Serial.println();
	Serial.println(F("-----------------------------------"));
	Serial.println(F("Application started"));
	Serial.println(F("-----------------------------------"));
	Serial.println();
}

// Set the brightness based on a correction for non linear brightness
// https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
void setLightBrightness(uint8_t value){
	uint8_t brightness = correctPwmBrightness(value);
	light.setBrightness(brightness);
}

// Function to correct for non linear brightness when using PWM
//
// Works but the light was practically off at brightness value 100
// Needs some sort of light calibration which is probably different for all bulbs, so disabled for now
uint8_t correctPwmBrightness(uint8_t value){
	return value;
	/*
	if (value == 255) {
		return 255;
	}
	return pow(2, (value / pwm)) - 1;
	*/
}

// Callback for when remote update starts
void remoteUpdateStarted() {
	Serial.println(F("UPDATE: HTTP update process started"));
}

// Callback for when remote update completes
void remoteUpdateFinished() {
	Serial.println(F("UPDATE: HTTP update process finished"));

	// Store current lamp state so that we can restart and return to what the lamp was doing
	config.systemUpdated();

	// Turn light off before restarting because the light (timer) becomes unstable
	setLightBrightness(0);
}

// Info on using the updater class:
// https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
// https://github.com/esp8266/Arduino/blob/16319da63d4e8f7b4d029bfe667ed1b370a485bb/doc/ota_updates/readme.rst
// 
// My changes to the ESP8266 WebUpdater:
//
// 1. New method on web updater to prevent an update if we just want to check if one is available
t_httpUpdate_return update(bool install){

	// Flash our built in LED
	ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

	// Set if we actually want to install the update if there is one
	ESPhttpUpdate.installUpdate(install);

	// If we're installing the update, set a callback to let the main program know the update has finished
	if (install){
		ESPhttpUpdate.onStart(remoteUpdateStarted);
		ESPhttpUpdate.onEnd(remoteUpdateFinished);
	}
	
	// Old way - works perfectly but is deprecated 
	t_httpUpdate_return ret = ESPhttpUpdate.updateNew(HTTP_UPDATE_DEVICE_ID, HTTP_UPDATE_SERVER, 80, HTTP_UPDATE_PATH);

	// New way - no worky, gives me a 301 (moved permenantly error), probably my host
	/*
	WiFiClient client;
	t_httpUpdate_return ret = ESPhttpUpdate.update(client, "http://supercrab.co.uk/update");
	*/

	// Handle returned result
	switch(ret) {
		case HTTP_UPDATE_AVAILABLE: 

			// This code is only executed when install = false
			// If install = true, then the system is updated and rebooted before it gets here
			Serial.println(F("UPDATE: available!"));
			break;

		case HTTP_UPDATE_FAILED:

			// Update was available but it failed
			Serial.println(F("UPDATE: failed!"));
			Serial.println(ESPhttpUpdate.getLastErrorString());
			if (install){
				config.setUpdateStatusFailed(ESPhttpUpdate.getLastErrorString());
			}
			break;

		case HTTP_UPDATE_NO_UPDATES:

			// There are not updated
			Serial.println(F("UPDATE: not available"));
			if (install){
				config.setUpdateStatusNoUpdates();
			}
			break;

		case HTTP_UPDATE_OK:
			// Here for completeness as this is not called because
			// a successful update causes the system to be rebooted)
			break;
	}
	return ret;
}

// Pulse the light 
void pulseLight(uint8_t times = 1){

	for (uint8_t t = 0; t < times; t++){

		// Ramp brightness up
		for (uint8_t b = 0; b < 255; b++){
			light.setBrightness(b);
			delay(6);
		}

		// Wait a bit at maximum
		delay(250);

		// Ramp brightness down
		for (uint8_t b = 255; b > 0; b--){
			light.setBrightness(b);
			delay(6);
		}
	}
}

// Setup hostname
void setupHostname(){

	Serial.print(F("WIFI: setting up hostname: ")); Serial.println(HOSTNAME);

	// Set the network host name
	WiFi.hostname(HOSTNAME);
}

// Callback when connecting to previous WiFi fails
void wifiManagerCallback(AsyncWiFiManager *myWiFiManager) {

	Serial.println(F("WIFI: entered configuration mode, creating access point"));
	Serial.println(WiFi.softAPIP());
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Setup MDNS
void setupMDNS(){

	// Try and give us a name MDNS
	Serial.print(F("MDNS: setting up host as ")); 
	Serial.print(HOSTNAME);
	Serial.println(F(".local"));

	MDNS.close();
	if (!MDNS.begin(HOSTNAME)) {
		Serial.println(F("MDNS: failed"));
	}
	else{
		// Broadcast our http web server on admin port
		MDNS.addService("http", "tcp", ADMIN_WEB_PORT);
		Serial.println(F("MDNS: OK!"));
	}
}

// Setup Wifi
void setupWifiManager() {

	Serial.println(F("WIFI: starting connection manager"));

	// AsyncWiFiManager intialization. Once its business is done, there is no need to keep it around
	AsyncWiFiManager wifiManager;

	// set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
	wifiManager.setAPCallback(wifiManagerCallback);

	// fetches ssid and pass and tries to connect
	// if it does not connect it starts an access point with the specified name
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager.autoConnect(ACCESS_POINT_NAME)){

		Serial.println(F("WIFI: failed to connect and hit timeout"));

		// reset and try again, or maybe put it to deep sleep
		ESP.restart();
		delay(1000);
	} 
	else {
		// The wifi manager has connected us!
		Serial.printf("WIFI: OK!  SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
	}
}

// Setup rotary encoder button
void setupButton(){
	button.begin();
}

// Setup rotary encoder
void setupRotaryEncoder(){
	setEncoderValue();
}

// Setup dimmer
void setupDimmer(){
	static bool dimmerInitialised = false;

	// only update dimmer if not updated
	if (!dimmerInitialised){
		DimmableLight::setSyncPin(ZERO_CROSS_PIN);
		DimmableLight::begin();
		dimmerInitialised = true;
	}
}

// Setup fauxmo library
void setupFauxmo(){

	fauxmo.createServer(true); 
	fauxmo.setPort(80); // for Gen3 devices
	fauxmo.setRedirect(ADMIN_WEB_PORT); // custom method to direct HTTP traffic on port 80 to custom port
	fauxmo.enable(true);

	// Add virtual device
	fauxmo.addDevice(config.getDeviceName());

	// Lamda expresison to handle light control  
	fauxmo.onSetState([](byte deviceId, const char* deviceName, bool deviceState, unsigned char value){

		// Output what we received
		Serial.printf("FAUXMO: device ID #%d (%s) state: %s value: %d\n", deviceId, deviceName, deviceState ? "ON " : "OFF", value);
		
		// Is the device matched?
		if (strcmp(deviceName, config.getDeviceName()) == 0) {

			// Store light state
			config.setState(deviceState);

			// Store light brightness
			config.setBrightness(value);

			// Set light brightness
			setLightBrightness(config.getState() ? config.getBrightness() : 0);

			// Set the rotary encolder value to current brightness (invert if required)
			setEncoderValue();
		} 
	});

	Serial.print(F("FAUXMO: initiated device called: "));
	Serial.println(config.getDeviceName());
}

// Setup webserver
void setupWebServer(){
	server.begin(&config);
}

// Setup a timer to periodically check wifi status
void setupWifiCheckTimer(){

	// Lambda expersion  to check wifi status is connected 
	timerWifiCheck.setCallback([](void){
		
		Serial.print("WIFI STATUS: ");

		// Check wifi status
		if (WiFi.status() != WL_CONNECTED){

			Serial.println("BAD!  Restarting device...");

			// Restart board and try and reconnect or enter AP mode
			ESP.restart();

			// Wait a while for the restart to kick in
			delay(5000);
		}
		else{
			Serial.println("OK");
		}
	});

	// Set the timer interval and start
	timerWifiCheck.setIntervalMinutes(config.getWifiCheckInterval()); 
	timerWifiCheck.start();
}

// Setup a timer to periodically check for automatic updates
void setupAutomaticUpdateTimer(){

	// Lambda expersion to run automatic update check
	timerAutomaticUpdate.setCallback([](void){

		// Check update - if update does occur the system is automatically restarted
		Serial.println(F("AUTO UPDATE: checking!"));
		updateSystem();
	});

	// Set the timer interval and start
	timerAutomaticUpdate.setIntervalHours(config.getAutomaticUpdates() ? AUTO_UPDATE_CHECK_PERIOD_HOURS : 0); 
	timerAutomaticUpdate.start();
}

// Reboot the system
void rebootSystem(){

	Serial.println(F("Rebooting..."));

	// Disable server
	server.end();
	
	// Wait for web connections to stop
	delay(250);

	// Restart
	ESP.restart();

	// Wait for restart to kick in
	delay(5000);
}

// Check and update system
void updateSystem(){

	Serial.println(F("UPDATE SYSTEM: checking and installing updates..."));

	// If we are doing an automatic update then we need to set our update status
	// This is also set in the web server layer when user visits the updatenow webpage
	config.setUpdateStatusRemoteUpdate();

	// Wait for any web connections to finish
	delay(250);

	// If update does occur the system is automatically restarted
	update(true);
}

// Update the light
void updateLight(){

	// Set the brightness!
	setLightBrightness(config.getState() ? config.getBrightness() : 0);

	// Reset rotary encoder to current brightness
	// (this prevents rotary encoder affecting brightness when light is off)
	setEncoderValue();

	// Broadcast light state
	updateAlexa();
}

// Broadcast light state to alexa
void updateAlexa(){

	// Only update fauxmo if wifi is enabled
	if (config.getMode() == WIFI){
		fauxmo.setState(config.getDeviceName(), config.getState(), config.getBrightness());
	}
}

// Update settings - called when web server adjusts settings
void updateSettings(){

	// Update light just in case state or brightness has been changed
	updateLight();

	if (config.getMode() == WIFI){

		// Adjust timers
		timerWifiCheck.setIntervalMinutes(config.getWifiCheckInterval());
		timerAutomaticUpdate.setIntervalHours(config.getAutomaticUpdates() ? AUTO_UPDATE_CHECK_PERIOD_HOURS : 0); 

		// Adjust fauxmo name
		fauxmo.renameDevice((byte) 0, config.getDeviceName());
	}
	else {

		// Prevent WIFI auto connecting when device is restarted
		WiFi.setAutoConnect(false);

		// Disable timers
		timerWifiCheck.stop();
		timerAutomaticUpdate.stop();

		// Stop fauxmo
		fauxmo.enable(false);

#ifdef ENABLE_MDNS
		// Stop MDNS
		MDNS.close();
#endif
		// Destroy web server
		server.end();
	}
}

// Invert encoder direction
void invertEncoder(){
	encoderOldValue = 255 - encoder.read();
	encoder.write(encoderOldValue);
}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------
void setup() {

	// This line disables the automatic ESP access point, e.g. ESP-xxxxx
	WiFi.mode(WIFI_STA);

	// https://github.com/me-no-dev/ESPAsyncWebServer/issues/716
	// ISSUE: using char arrays causes resets
	// FIX: prevent re-use of the OS stack
	disable_extra4k_at_link_time();

	setupSerial();

	// Initialse/load settings from EEPROM
	config.begin();

	// Button setup
	setupButton();

	// Dimmer setup
	setupDimmer();

	// Pass methods into config which will update the main thread 
	config.onLightChanged(updateLight);
	config.onSettingsChanged(updateSettings);
	config.onEncoderInverted(invertEncoder);

	// Change mode?
	if (button.read()){

		// Flip wifi mode
		config.toggleMode();

		// Save the settings
		config.save();

		// If WIFI has been disabled then prevent WIFI auto connecting when device is restarted
		if (config.getMode() != WIFI){
			WiFi.setAutoConnect(false);
		}

		// Pulse light to (twice = wifi on, once = wifi off)
		pulseLight(config.getMode() == WIFI ? 2 : 1);

		// Clear the previous button press!
		setupButton();
	}

	// Is light in Wifi mode?
	if (config.getMode() == WIFI) {

		Serial.println(F("WIFI: starting setup"));

		// Setup hostname
		setupHostname();

		// Setup wifi manager
		setupWifiManager();

#ifdef ENABLE_MDNS
		// Setup MSDNS
		// This takes a couple of minutes to work if the Wifi manager has been invoked
		// and the device has just configure to connect to a new WIFI network
		setupMDNS();
#endif

		// Setup fauxmo
		setupFauxmo();

		// Setup webserver
		setupWebServer();

		// Setup the wifi connected check
		setupWifiCheckTimer();

		// Setup automatic update check
		setupAutomaticUpdateTimer();
	}
	else {
		Serial.println(F("WIFI: disabled"));
	}

	// Setup rotary encoder (this caused issues when called sooner)
	setupRotaryEncoder();

	// Lamp is on after a system update so broadcast its state and update encoder
	if (config.getState()){
		updateLight();
	}

	Serial.println(F("SETUP: finished"));
}

// Set the encoder value depending on current brightness and encoder inverted setting
void setEncoderValue(){

	// Setting the encoder value prevents an uncessary change in the encoder being picked up in the main loop
	encoderOldValue = config.getEncoderInverted() ? 255 - config.getBrightness() : config.getBrightness();
	encoder.write(encoderOldValue);
}

// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------
void loop() {

	// Do we need to reboot the system after a firmware upload or reboot call?
	// Reboot needs to be called from outside of the web request handling
	if (config.getRebootSystem()){
		rebootSystem();
	}

	// Have we been asked to do an upate by the web server?
	if (config.getUpdateStatus() == UPDATE_STATUS_REMOTE_UPDATE){
		updateSystem();
	}

	// Is the light in WiFi mode?
	if (config.getMode() == WIFI){

#ifdef ENABLE_MDNS
		// Need this for MDNS to work
		MDNS.update();
#endif
		// Periodically check if Wifi is connected 
		timerWifiCheck.handle();

		// Periodically check if any updates are available
		timerAutomaticUpdate.handle();

		// fauxmoESP uses an async TCP server but a sync UDP server
		// Therefore, we have to manually poll for UDP packets
		fauxmo.handle();
	}

	// Read button presses
	button.read();

	// Turn light on or off if button has been pressed
	if (button.wasPressed()){

		// Flip light state
		config.toggleState();

		// Set the brightness!
		setLightBrightness(config.getState() ? config.getBrightness() : 0);

		// Reset rotary encoder to current brightness
		// (this prevents rotary encoder affecting brightness when light is off)
		setEncoderValue();

		// Broadcast light state
		updateAlexa();

		Serial.printf("Button pressed!  State: %s value: %d\n", config.getState() ? "ON" : "OFF", config.getBrightness());
	}

	// If light is on allow rotary encoder to change brightness
	if (config.getState()){

		// Calculate new value with sensitivity factor
		int16_t encoderNewValue = encoderOldValue - ((encoderOldValue - encoder.read()) * config.getEncoderSensitivity());

		// Enforce rotary encoder maximum value
		if (encoderNewValue > 255){
			encoderNewValue = 255;
			encoder.write(encoderNewValue);	
		}

		// Enforce rotary encoder minimum value
		else if (encoderNewValue < 0){
			encoderNewValue = 0;
			encoder.write(encoderNewValue);	
		}

		// Rotary encoder has changed so change brightness
		if (encoderNewValue != encoderOldValue) {

			// Set encoder value if not on default sensitivity
			if (config.getEncoderSensitivity() > 1){
				encoder.write(encoderNewValue);	
			}

			// Store brightness depending on whether we need to invert the encoder
			config.setBrightness(config.getEncoderInverted() ? 255 - encoderNewValue : encoderNewValue);

			// Set light brightness
			setLightBrightness(config.getBrightness());

			// Broadcast light state
			updateAlexa();

			// Store the old value
			encoderOldValue = encoderNewValue;
		}
	}
}