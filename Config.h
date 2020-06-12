#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <EEPROM.h>
#include "Common.h"

struct bag_t {
	char deviceName[ALEXA_NAME_LENGTH];
	uint8_t mode;
	bool encoderInverted;
	uint8_t encoderSensitivity;
	uint8_t wifiCheckInterval;
	bool automaticUpdates;
	bool initialState;
	uint8_t initialBrightness;
};

// Class to hold configuration settings
class Config{

	public:
		Config():
			_state(INITIAL_STATE), 
			_brightness(INITIAL_BRIGHTNESS)
			{}

		void begin();

		void onLightChanged(void(*lightCallback)(void));
		void onSettingsChanged(void(*settingsCallback)(void));
		void onEncoderInverted(void(*invertEncoderCallback)(void));

		bool load();
		bool save();

		char* getDeviceName();
		void setDeviceName(char* name);
		uint8_t getDeviceNameLength();

		operation_t getMode();
		void setMode(operation_t mode);
		void toggleMode();

		bool getState();
		void setState(bool state);
		void toggleState();

		uint8_t getBrightness();
		void setBrightness(uint8_t brightness);

		bool getEncoderInverted();
		void setEncoderInverted(bool inverted);

		uint8_t getEncoderSensitivity();
		void setEncoderSensitivity(uint8_t sensitivity);

		uint8_t getWifiCheckInterval();
		void setWifiCheckInterval(uint8_t interval);

		bool getAutomaticUpdates();
		void setAutomaticUpdates(bool updates);

		updateStatus_t getUpdateStatus();
		bool getIsUpdating();

		void setUpdateStatusIdle();
		void setUpdateStatusNoUpdates();
		void setUpdateStatusLocalUpdate();
		void setUpdateStatusRemoteUpdate();
		void setUpdateStatusFailed(String message);
		void setUpdateStatusOK();

		String getUpdateMessage();

		void updateLight();
		void updateSettings();

		bool getRebootSystem();
		void setRebootSystem();

		void systemUpdated();

	private:
		operation_t _mode;
		uint8_t _state;
		uint8_t _brightness;
		bag_t	_bag; 
		bool	_updateSystem;
		bool	_rebootSystem;

		updateStatus_t _updateStatus;
		String _updateMessage;

		void (*_updateLightCallback)(void) = NULL;
		void (*_updateSettingsCallback)(void) = NULL;
		void (*_invertEncoderCallback)(void) = NULL;

		void _outputSettings();
		void _initialisePreviousState();
};

#endif