#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <EEPROM.h>

// Initial lamp brightness when turned on for the first time
#define INITIAL_BRIGHTNESS 255
// How often should we check wifi is connected? (0 = never check)
#define WIFI_CONNECTED_CHECK_MINS 1
// How sensitive the encoder is 
// (1 = slowest, higher means more change)
#define ENCODER_SENSITIVITY 2

// Class to hold configuration settings
class Config{
	public:
		Config():
			_wifi(true), // Wifi always on by default
			_state(false), // Lamp always off by default
			_brightness(INITIAL_BRIGHTNESS),
			_wifiCheckInterval(WIFI_CONNECTED_CHECK_MINS),
			_encoderSensitivity(ENCODER_SENSITIVITY){}

		bool load();
		bool save();

		bool getWifi();
		void setWifi(bool enabled);
		void invertWifi();

		bool getState();
		void setState(bool state);
		void invertState();

		uint8_t getBrightness();
		void setBrightness(uint8_t brightness);

		uint8_t getEncoderSensitivity();
		void setEncoderSensitivity(uint8_t sensitivity);

		uint8_t getWifiCheckInterval();
		void setWifiCheckInterval(uint8_t interval);
	private:
		uint8_t _wifi;
		uint8_t _state;
		uint8_t _brightness;
		uint8_t _encoderSensitivity;
		uint8_t _wifiCheckInterval;
};

#endif