#ifndef AdminWebServer_h
#define AdminWebServer_h

#include <ESPAsyncWebServer.h>

#include "Common.h"
#include "Config.h"
#include "StringStream.h"

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "Favicon.h"
#include "HtmlUpdatingPage.h"
#include "HtmlFormElements.h"
#include "HtmlAboutPage.h"
#include "HtmlHomePage.h"
#include "HtmlUpdateErrorPage.h"
#include "HtmlUpdatePage.h"
#include "HtmlErrorPage.h"
#include "HtmlSettingsPage.h"

// Html for selected option in form select element
const char* const HTML_SELECTED = "selected";

// Length of selected option 
const uint8_t HTML_SELECTED_LENGTH = strlen(HTML_SELECTED);

// Number of characters an integer value in an input element can take
const uint8_t HTML_VALUE_LENGTH = 4; // 3 digits + null

// Simple JSON response
const char JSON_SETTING_RESPONSE[] PROGMEM = R"rawliteral({"value":%d})rawliteral";

// Administration web server
class AdminWebServer{
	
	public:
		AdminWebServer(uint16_t port) : _server(port){}
		void begin(Config* config);
		void end();

	private:
		AsyncWebServer _server;
		Config* _config;
		String _getStrFromEndOfString(const String s);
		int16_t _getIntFromEndOfString(const String s);
		const char* _getVersion();
		const char* _getJSONsettingIntResponse(const uint8_t value);
		const char* _getUptime();
		bool _systemUpdatingRedirect(AsyncWebServerRequest *request);
};

#endif