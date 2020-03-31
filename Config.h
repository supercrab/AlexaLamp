#ifndef Config_h
#define Config_h

#include <Arduino.h>

// Struct to hold config values
struct Config{
	bool wifi;
	bool state;
	uint8_t brightness;
	uint8_t encoderSensitivity;
	uint8_t wifiCheckInterval;
};

#endif