/*	____ _    ____ _  _ ____       _    ____ _  _ ___  
 *	|__| |    |___  \/  |__|       |    |__| |\/| |__] 
 *	|  | |___ |___ _/\_ |  |       |___ |  | |  | |    
 *				with brightness control
 *
 *	by Supercrab
 *	https://github.com/supercrab
 *
 *	Developed on a WeMos D1 mini clone, using Arduino IDE 1.6.5 with ESP8266 Core 2.3.0
 *
 *	This project would not have been possible without the kind development of others on GitHub!
 */

// For this and that, me and you
#include <Arduino.h>
#include <EEPROM.h>

// ---------------------------------------------------------
//	* Fauxmoesp (Xose Pérez, Ben Hencke)
//	https://github.com/simap/fauxmoesp
// ---------------------------------------------------------
#if defined(ESP8266)
	#include <ESP8266WiFi.h>
	#include <ESPAsyncTCP.h>
#elif defined(ESP32)
	#include <WiFi.h>
	#include <AsyncTCP.h>
#else
	#error Platform not supported at the moment because of the fauxmo library
#endif
#include <fauxmoESP.h>

// ---------------------------------------------------------
//	* Dimmable-Light-Arduino (Fabiano Riccardi)
//	https://github.com/fabiuz7/Dimmable-Light-Arduino
// ---------------------------------------------------------
#include <dimmable_light.h>

// ---------------------------------------------------------
//	* WifiManager (tzapu)
//	https://github.com/tzapu/WiFiManager
// ---------------------------------------------------------
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

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
// Project libraries
// ---------------------------------------------------------
#include "SoftwareTimer.h"
#include "Config.h"
#include "AdminWebServer.h"

// ---------------------------------------------------------
// Software settings
// ---------------------------------------------------------

// Serial baudrate 
#define SERIAL_BAUDRATE 115200
// Admin port for admin web page
#define ADMIN_WEB_PORT 81

// Device name for Alexa and also access point name for WifiManager
#define DEVICE_NAME     "Lamp"

// ---------------------------------------------------------
// Hardware settings
// ---------------------------------------------------------

// Zero cross detection pin
#define ZERO_CROSS_PIN	D1
// Triac pin 
#define TRIAC_PIN		D2 

// Encoder switch
#define ENCODER_SWITCH_PIN	D5
// Encoder A pin
#define ENCODER_A_PIN	D6
// Encode B pin 
#define ENCODER_B_PIN	D7

fauxmoESP fauxmo;
DimmableLight light(TRIAC_PIN);
Encoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
Button button(ENCODER_SWITCH_PIN);
SoftwareTimer timer;
AdminWebServer server(ADMIN_WEB_PORT);
Config config;

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

// Callback when connecting to previous WiFi fails
void wifiManagerCallback(WiFiManager *myWiFiManager) {
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Setup Wifi
void setupWifiManager() {
	// WiFiManager intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;
	//set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
	wifiManager.setAPCallback(wifiManagerCallback);
	// fetches ssid and pass and tries to connect
	// if it does not connect it starts an access point with the specified name
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager.autoConnect(DEVICE_NAME)) {
		Serial.println("failed to connect and hit timeout");
		//reset and try again, or maybe put it to deep sleep
		ESP.restart();
		delay(1000);
	} 
	else {
		// The wifi manager has connected us!
		Serial.printf("Wifi connected!  SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
	}
}

// Setup rotary encoder button
void setupButton(){
	button.begin();
}

// Setup rotary encoder
void setupRotaryEncoder(){
	encoder.write(config.getBrightness() / config.getEncoderSensitivity());
}

// Setup dimmer
void setupDimmer(){
	static bool dimmerInitialised = false;
	if (!dimmerInitialised){
		DimmableLight::setSyncPin(ZERO_CROSS_PIN);
		DimmableLight::begin();
		dimmerInitialised = true;
	}
}

// Setup fauxmo library
void setupFauxmo(){
	// By default, fauxmoESP creates it's own webserver on the defined port
	// The TCP port must be 80 for gen3 devices (default is 1901)
	// This has to be done before the call to enable()
	fauxmo.createServer(true); // not needed, this is the default value
	fauxmo.setPort(80); // This is required for gen3 devices
	// You have to call enable(true) once you have a WiFi connection
	// You can enable or disable the library at any moment
	// Disabling it will prevent the devices from being discovered and switched
	fauxmo.enable(true);
	// You can use different ways to invoke alexa to modify the devices state:
	// "Alexa, turn yellow lamp on"
	// "Alexa, turn on yellow lamp
	// "Alexa, set yellow lamp to fifty" (50 means 50% of brightness, note, this example does not use this functionality)
	
	// Add virtual device
	fauxmo.addDevice(DEVICE_NAME);
	fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool device_state, unsigned char value){
		// Callback when a command from Alexa is received. 
		// You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
		// State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
		// Just remember not to delay too much here, this is a callback, exit as soon as possible.
		// If you have to do something more involved here set a flag and process it in your main loop.
		Serial.printf("Fauxmo: Device ID #%d (%s) state: %s value: %d\n", device_id, device_name, device_state ? "ON " : "OFF", value);

		// Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
		// Otherwise comparing the device_name is safer.
		if (strcmp(device_name, DEVICE_NAME) == 0) {
			// Store lamp state brightness	
			config.setState(device_state);
			config.setBrightness(value);
			// Set lamp brightness
			light.setBrightness(config.getState() ? config.getBrightness() : 0);
			// Set the rotary encolder value to current brightness
			encoder.write(config.getBrightness() / config.getEncoderSensitivity());
		} 
	});

	Serial.println("Fauxmo: started!");
}

// Setup webserver
void setupWebServer(){
	server.begin(&config);
}

// Setup a timer to periodically check wifi status
void setupCheckWifiTimer(){
	// Lambda expersion  to check wifi status is connected 
	timer.setCallback([](void){
		Serial.print("Wifi connection: ");
		// Check wifi status
		if (WiFi.status() != WL_CONNECTED){
			// Restart board and try and reconnect or enter AP mode
			Serial.println("BAD!  Restarting device...");
			ESP.restart();
			delay(5000);
		}
		else{
			Serial.println("OK");
		}
	});
	timer.setIntervalMins(config.getWifiCheckInterval()); 
	timer.start();
}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------
void setup() {

	// Init serial port and clean garbage
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println();
	Serial.println();

	// Button setup
	setupButton();
	// Dimmer setup
	setupDimmer();

	// Change mode?
	if (button.read()){
		// Flip wifi mode
		config.invertWifi();
		// Save setting
		config.save();
		// Pulse lamp (twice = wifi on, once = wifi off)
		pulseLight(config.getWifi() ? 2 : 1);
		// Clear the previous button press!
		setupButton();
	}

	// Is lamp in Wifi mode?
	if (config.getWifi()) {
		// Setup wifi manager
		setupWifiManager();
		// Setup fauxmo
		setupFauxmo();
		// Setup webserver
		setupWebServer();
		// Setup the wifi connected check
		setupCheckWifiTimer();
	}

	// Setup rotary encoder (this caused issues when called sooner)
	setupRotaryEncoder();
}

// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------
void loop() {

	// Is the lamp in WiFi mode?
	if (config.getWifi()){
		// Periodically check if Wifi is connected 
		timer.handle();
		// fauxmoESP uses an async TCP server but a sync UDP server
		// Therefore, we have to manually poll for UDP packets
		fauxmo.handle();
		//webhandle();
		server.handle();
	}

	// Read button presses
	button.read();

	// Turn lamp on or off if button has been pressed
	if (button.wasPressed()){
		// Flip lamp state
		config.invertState();
		// Set the brightness!
		light.setBrightness(config.getState() ? config.getBrightness() : 0);
		// Reset rotary encoder to current brightness
		// (this prevents rotary encoder affecting brightness when lamp is off)
		encoder.write(config.getBrightness() / config.getEncoderSensitivity());
		// Broadcast lamp state
		if (config.getWifi()){
			fauxmo.setState(DEVICE_NAME, config.getState(), config.getBrightness());
		}
		Serial.printf("Button pressed! State: %s value: %d\n", config.getState() ? "ON " : "OFF", config.getBrightness());
	}

	// If lamp is on allow rotary encoder to change brightness
	if (config.getState()){
		// Previous encoder value
		static int16_t encoderOldValue = 0;
		// Calculate new value with sensitivity factor
		int16_t encoderNewValue = encoder.read() * config.getEncoderSensitivity();
		// Enforce rotary encoder maximum value
		if (encoderNewValue > 255){
			encoderNewValue = 255;
			encoder.write(encoderNewValue / config.getEncoderSensitivity());
		}
		// Enforce rotary encoder minimum value
		else if (encoderNewValue < 0){
			encoderNewValue = 0;
			encoder.write(encoderNewValue);	
		}
		// Rotary encoder has changed so change brightness
		if (encoderNewValue != encoderOldValue) {
			encoderOldValue = encoderNewValue;
			config.setBrightness(encoderNewValue);
			// Set lamp brightness
			light.setBrightness(config.getBrightness());
			// Broadcast lamp state
			if (config.getWifi()) {
				fauxmo.setState(DEVICE_NAME, true, config.getBrightness());
			}
		}
	}
}