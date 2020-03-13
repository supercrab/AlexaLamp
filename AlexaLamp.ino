/*
	____ _    ____ _  _ ____       _    ____ _  _ ___  
	|__| |    |___  \/  |__|       |    |__| |\/| |__] 
	|  | |___ |___ _/\_ |  |       |___ |  | |  | |    
	with brightness control

	by Supercrab
	https://github.com/supercrab

	Developed on a WeMos D1 mini clone, using Arduino IDE 1.6.5 with ESP8266 Core 2.3.0

	This project would not have been possible without the kind development of others on GitHub!

	Required libraries & credits
	============================

	* Fauxmoesp (Xose Pérez, Ben Hencke)
	https://github.com/simap/fauxmoesp

	* Encoder (Paul Stoffregen)
	https://github.com/PaulStoffregen/Encoder

	* WifiManager (tzapu)
	https://github.com/tzapu/WiFiManager

	* JC_Button (Jack Christensen)
	https://github.com/JChristensen/JC_Button

	* Dimmable-Light-Arduino (Fabiano Riccardi)
	https://github.com/fabiuz7/Dimmable-Light-Arduino
*/
#include <Arduino.h>

// Device constraints due to fauxmo library
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

// Lamp brightness control
#include <dimmable_light.h>

// WifiManager libraries
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Encoder library using internal pullups
#include <Encoder.h>

// Button library
#include <JC_Button.h>

// User settings!  Yes there are some!
#include <EEPROM.h>

// Serial baudrate 
#define SERIAL_BAUDRATE 115200

// Device and access point name for WifiManager
#define DEVICE_NAME     "Lamp"
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

// How sensitive the encoder is 
// (1 = slowest, higher means more change)
#define ENCODER_SENSITIVITY 2

fauxmoESP fauxmo;
DimmableLight light(TRIAC_PIN);
Encoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
Button button(ENCODER_SWITCH_PIN);

// Initial lamp brightness when turned on
uint8_t brightness = 255;
// Lamp state
uint8_t state = false;
// Enable wifi mode (default is true)
bool wifi = true;

// Read any settings from EEPROM
void readSettings(){

	// Initialise EEPROM
	EEPROM.begin(512);
	// check for previous settings 
	if (EEPROM.read(0) == 'L' && EEPROM.read(1) == 'A' && EEPROM.read(2) == 'M' && EEPROM.read(3) == 'P'){
		wifi = EEPROM.read(4);
		Serial.printf("Found previous settings! Wifi enabled: %s\n", wifi ? "YES" : "NO" );
	}
	else
		Serial.println("No previous settings found - using default!");
}

// Store our settings
void saveSettings(){
	// Write our special string
	EEPROM.write(0, 'L');    
	EEPROM.write(1, 'A');   
	EEPROM.write(2, 'M');
	EEPROM.write(3, 'P');
	EEPROM.write(4, wifi);
	// Save settings
	EEPROM.commit();
	Serial.printf("Saved settings! Wifi enabled: %s\n", wifi ? "YES" : "NO" );
}

// Callback when connecting to previous WiFi fails
void configModeCallback(WiFiManager *myWiFiManager) {
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Setup Wifi
void setupWifi() {
	
	// WiFiManager intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;
	// reset settings - for testing
	//wifiManager.resetSettings();

	//set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
	wifiManager.setAPCallback(configModeCallback);

	// fetches ssid and pass and tries to connect
	// if it does not connect it starts an access point with the specified name
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager.autoConnect(DEVICE_NAME)) {
		Serial.println("failed to connect and hit timeout");
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(1000);
	} 
	else {
		// The wifi manager has connected us!
		Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
	}
}

// Setup rotary encoder button
void setupButton(){
	button.begin();
}

// Setup rotary encoder
void setupRotaryEncoder(){
	encoder.write(brightness / ENCODER_SENSITIVITY);
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

//Setup fauxmo library
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

	fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool device_state, unsigned char value) {
		// Callback when a command from Alexa is received. 
		// You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
		// State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
		// Just remember not to delay too much here, this is a callback, exit as soon as possible.
		// If you have to do something more involved here set a flag and process it in your main loop.
		Serial.printf("Device ID #%d (%s) state: %s value: %d\n", device_id, device_name, device_state ? "ON " : "OFF", value);

		// Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
		// Otherwise comparing the device_name is safer.
		if (strcmp(device_name, DEVICE_NAME) == 0) {

			// Store lamp state brightness	
			state = device_state;
			brightness = value;

			// Set lamp brightness
			light.setBrightness(state ? brightness : 0);

			// Set the rotary encolder value to current brightness
			encoder.write(brightness / ENCODER_SENSITIVITY);
		} 
	});

	Serial.println("Fauxmo has started!");
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

// Setup everything
void setup() {

	// Init serial port and clean garbage
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println();
	Serial.println();

	// Read previous settings
	readSettings();

	// Button setup
	setupButton();

	// Dimmer setup
	setupDimmer();

	// Change mode?
	if (button.read()){

		// Flip setting
		wifi = !wifi;

		// Save setting
		saveSettings();

		// Pulse lamp (twice = wifi on, once = wifi off)
		pulseLight(wifi ? 2 : 1);

		// Clear the previous button press!
		setupButton();
	}

	// Is lamp in Wifi mode?
	if (wifi) {

		// Setup wifi
		setupWifi();

		// Setup fauxmo
		setupFauxmo();
	}

	// Setup rotary encoder 
	// (this caused issues if it was called earlier)
	setupRotaryEncoder();
}

void loop() {

	// fauxmoESP uses an async TCP server but a sync UDP server
	// Therefore, we have to manually poll for UDP packets
	if (wifi) fauxmo.handle();

	// Read button presses
	button.read();

	// Turn lamp on or off if button has been pressed
	if (button.wasPressed()){

		// Flip lamp state
		state = !state;

		// Set the brightness!
		light.setBrightness(state ? brightness : 0);

		// Reset rotary encoder to current brightness
		// (this prevents rotary encoder affecting brightness when lamp is off)
		encoder.write(brightness / ENCODER_SENSITIVITY);

		// Broadcast lamp state
		if (wifi) fauxmo.setState(DEVICE_NAME, state, brightness);

		Serial.printf("Button pressed! State: %s value: %d\n", state ? "ON " : "OFF", brightness);
	}

	// If lamp is on allow rotary encoder to change brightness
	if (state){

		// Previous encoder value
		static int16_t encoderOldValue = 0;

		// Calculate new value with sensitivity factor
		int16_t encoderNewValue = encoder.read() * ENCODER_SENSITIVITY;

		// Enforce rotary encoder maximum value
		if (encoderNewValue > 255){
			encoderNewValue = 255;
			encoder.write(encoderNewValue / ENCODER_SENSITIVITY);
		}

		// Enforce rotary encoder minimum value
		else if (encoderNewValue < 0){
			encoderNewValue = 0;
			encoder.write(encoderNewValue);	
		}

		// Rotary encoder has changed so change brightness
		if (encoderNewValue != encoderOldValue) {
			encoderOldValue = encoderNewValue;
			brightness = encoderNewValue;

			// Set lamp brightness
			light.setBrightness(brightness);

			// Broadcast lamp state
			if (wifi) fauxmo.setState(DEVICE_NAME, true, brightness);
		}
	}
}