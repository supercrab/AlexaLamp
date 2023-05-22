#include "AdminWebServer.h"

// Retrieve a number of digits after the last /
int16_t AdminWebServer::_getIntFromEndOfString(const String s){
	int8_t lastSlash;
	int8_t length;
	int16_t value = -1;

	// Look for a number at the end of the url
	lastSlash = s.lastIndexOf("/");
	if (lastSlash > -1){
		length = s.length() - (lastSlash + 1);
		if (length > 0){
			value = s.substring(lastSlash + 1).toInt();
		}
	}
	return value;
}

// I did try to use a sscanf() but it wasn't supported out of the box on my version of the ESP core
// I found an implemention similiar to this https://github.com/skorokithakis/A6lib/blob/master/sscanf.cpp
// However: it seemed to crash my system when I used it
const char* AdminWebServer::_getVersion(){
	static char buffer[15]; // Has to be hard coded length because it's a static variable
	static char timestamp[9]; //hh:mm:ss0
	static const char monthNames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	static char year[5];
	static char day[3];
	uint8_t monthInt;
	uint8_t dayInt;

	// Format time to HHMM
	strcpy_P(timestamp, PSTR(__TIME__));
	timestamp[2] = timestamp[3];
	timestamp[3] = timestamp[4];
	timestamp[4] = 0;

	// Copy year to new string
	strcpy_P(buffer, PSTR(__DATE__));
	year[0] = buffer[7];
	year[1] = buffer[8];
	year[2] = buffer[9];
	year[3] = buffer[10];
	year[4] = 0;
	
	// Copy day to a new string
	day[0] = buffer[4];
	day[1] = buffer[5];
	day[2] = 0;
	dayInt = atoi(day);

	// Shove a null to mark the end of the month
	buffer[3] = 0;

	// Get the month number
	monthInt = (strstr(monthNames, buffer) - monthNames)/3+1;

	// Bundle all this together
	sprintf_P(buffer, PSTR("%s%02d%02d.%s"), year, monthInt, dayInt, timestamp);

	return buffer;
}

// Return JSON response when a setting has ben updated
const char* AdminWebServer::_getJsonSettingIntResponse(const uint8_t value){
	static char buffer[16]; // Has to be hard coded length because it's a static variable

	snprintf_P(buffer, sizeof(buffer), JSON_SETTING_RESPONSE, value);

	return buffer;
}

// Retrieve uptime formatted
const char* AdminWebServer::_getUpTime(){
	static char buffer[43]; // xxxx days, xx hours, xx mins, xx seconds

	uint32_t sec = millis() / 1000;
	uint32_t min = sec / 60;
	uint16_t hr = min / 60;
	uint16_t day = hr / 24;

	sprintf_P(buffer, PSTR("%d days, %d hours, %d mins, %d seconds"), day, hr % 24, min % 60, sec % 60);

	return buffer;
}

// Start the webserver and take config
void AdminWebServer::begin(Config* config){
	_config = config;

	// ------------------------------------------------------------------
	// HTML pages
	// ------------------------------------------------------------------

	// Homepage
	_server.on(PSTR("/"), HTTP_GET, [this](AsyncWebServerRequest *request){

		// Display updating page if system is being updated
		if (_systemUpdatingRedirect(request)){
			return;
		}

		// Display normal home page
		char html[HTML_VALUE_LENGTH + strlen_P(HTML_HOME_PAGE)];
		snprintf_P(
			html, sizeof(html),
			HTML_HOME_PAGE,
			_config->getBrightness()
		);
		request->send(200, F("text/html"), html);
	});

	// Settings
	_server.on(PSTR("/settings"), HTTP_GET, [this](AsyncWebServerRequest *request){

		// Display updating page if system is being updated
		if (_systemUpdatingRedirect(request)){
			return;
		}
		char html_mode[strlen_P(HTML_MODE_INPUT) + HTML_SELECTED_LENGTH];
		char html_invert[strlen_P(HTML_INVERT_INPUT) + HTML_SELECTED_LENGTH];
		char html_sensitivity[strlen_P(HTML_SENSITIVITY_INPUT) + HTML_SELECTED_LENGTH];
		char html_wifi_interval[strlen_P(HTML_WIFI_CHECK_INTERVAL_INPUT) + HTML_SELECTED_LENGTH];
		char html_automatic_updates[strlen_P(HTML_AUTOMATIC_UPDATES_INPUT) + HTML_SELECTED_LENGTH];
		char html[
			strlen_P(HTML_SETTINGS_PAGE) + 
			sizeof(_config->getDeviceName()) +
			sizeof(html_mode) +
			sizeof(html_invert) +
			sizeof(html_sensitivity) +
			sizeof(html_wifi_interval) +
			sizeof(html_automatic_updates)
		];

		// Create lamp mode combo
		snprintf_P(
			html_mode, sizeof(html_mode),
			HTML_MODE_INPUT,
			_config->getMode() == STANDALONE ? HTML_SELECTED : "",
			_config->getMode() == WIFI ? HTML_SELECTED : ""
		);

		// Create encoder invert combo
		snprintf_P(
			html_invert, sizeof(html_invert),
			HTML_INVERT_INPUT,
			!_config->getEncoderInverted() ? HTML_SELECTED : "",
			_config->getEncoderInverted() ? HTML_SELECTED : ""
		);

		// Create knob sensitivty combo
		snprintf_P(
			html_sensitivity, sizeof(html_sensitivity),
			HTML_SENSITIVITY_INPUT,
			_config->getEncoderSensitivity() == 1 ? HTML_SELECTED : "",
			_config->getEncoderSensitivity() == 2 ? HTML_SELECTED : "",
			_config->getEncoderSensitivity() == 3 ? HTML_SELECTED : "",
			_config->getEncoderSensitivity() == 4 ? HTML_SELECTED : "",
			_config->getEncoderSensitivity() == 5 ? HTML_SELECTED : "",
			_config->getEncoderSensitivity() == 6 ? HTML_SELECTED : ""
		);

		// Create wifi check interval combo
		snprintf_P(
			html_wifi_interval, sizeof(html_wifi_interval),
			HTML_WIFI_CHECK_INTERVAL_INPUT,
			_config->getWifiCheckInterval() == 0  ? HTML_SELECTED : "",
			_config->getWifiCheckInterval() == 1  ? HTML_SELECTED : "",
			_config->getWifiCheckInterval() == 5  ? HTML_SELECTED : "",
			_config->getWifiCheckInterval() == 15 ? HTML_SELECTED : "",
			_config->getWifiCheckInterval() == 30 ? HTML_SELECTED : "",
			_config->getWifiCheckInterval() == 60 ? HTML_SELECTED : ""
		);

		// Create automatic updates combo
		snprintf_P(
			html_automatic_updates, sizeof(html_automatic_updates),
			HTML_AUTOMATIC_UPDATES_INPUT,
			!_config->getAutomaticUpdates() ? HTML_SELECTED : "",
			_config->getAutomaticUpdates() ? HTML_SELECTED : ""
		);

		// Populate page with controls
		snprintf_P(
			html, sizeof(html),
			HTML_SETTINGS_PAGE,
			_config->getDeviceName(), _config->getDeviceNameLength(),
			html_mode,
			html_invert,
			html_sensitivity,
			html_wifi_interval,
			html_automatic_updates,
			_config->getDeviceName()
		);
	
		request->send(200, F("text/html"), html);
	});

	// Check for updates page
	_server.on(PSTR("/updatenow"), HTTP_GET, [this](AsyncWebServerRequest *request){
		request->send_P(200, F("text/html"), HTML_UPDATING_PAGE);
		_config->setUpdateStatusRemoteUpdate();
	});

	// About page
	_server.on(PSTR("/about"), HTTP_GET, [this](AsyncWebServerRequest *request){

		// Display updating page if system is being updated
		if (_systemUpdatingRedirect(request)){
			return;
		}
		const char* uptime = _getUpTime();

		// Create array for our html
		char html[
			strlen_P(HTML_ABOUT_PAGE) +
			strlen(_getVersion()) +
			strlen(WiFi.macAddress().c_str()) +
			strlen(uptime) +
			strlen(ESP.getSketchMD5().c_str()) +
			strlen(__DATE__) + 
			strlen(__TIME__) + 4 // FreeHeap KB + \0
		];

		// Print our data into the char array
		snprintf_P(
			html, sizeof(html),
			HTML_ABOUT_PAGE,
			_getVersion(),
			WiFi.macAddress().c_str(),
			uptime,
			ESP.getSketchMD5().c_str(),
			__DATE__,
			__TIME__,
			ESP.getFreeHeap() / 1024
		);

		// Return html
		request->send(200, F("text/html"), html);
	});

	// Firmware update page
	_server.on(PSTR("/update"), HTTP_GET, [this](AsyncWebServerRequest *request){

		// Display updating page if system is being updated
		if (_systemUpdatingRedirect(request)){
			return;
		}

		// Create array for our html
		char html[
			strlen_P(HTML_UPDATE_PAGE) +
			strlen(HTTP_UPDATE_DEVICE_ID) +
			strlen(ESP.getSketchMD5().c_str()) 
		];

		// Print our data into the char array
		snprintf_P(
			html, sizeof(html),
			HTML_UPDATE_PAGE,
			HTTP_UPDATE_DEVICE_ID, 
			ESP.getSketchMD5().c_str()
		);

		// Return our html
		request->send(200, F("text/html"), html);
	});

	// Handle the update page being posted
	// https://github.com/me-no-dev/ESPAsyncWebServer
	_server.on(PSTR("/update"), HTTP_POST,
		// Respond to the posted file (done last after file has been uploaded)
		[this](AsyncWebServerRequest *request){
			char html[strlen_P(HTML_UPDATE_ERROR_PAGE) + 128];
			bool reboot = false;
			AsyncWebServerResponse *response;

			// The HTML form prevents us from submitting a blank file but it would still
			// be treated as a valid choice be the updater and no error would occur
			if (!Update.hasError()){

				// Flag for telling app that we need to reboot
				reboot = true;

				// Set the update status and message
				_config->setUpdateStatusOK();

				// Start response
				response = request->beginResponse_P(200, F("text/html"), HTML_UPDATING_PAGE);
			}
			else{
				// Create a stream to retrieve the error message
				String errorMessage;
				StringStream stream((String&) errorMessage);
				Update.printError(stream);

				// Store the error message
				_config->setUpdateStatusFailed(errorMessage);

				// Output the error message
				snprintf_P(
					html, sizeof(html),
					HTML_UPDATE_ERROR_PAGE,
					_config->getUpdateMessage().c_str()
				);

				// Start response
				response = request->beginResponse(200, F("text/html"), html); 
			}

			// Send the response to client
			response->addHeader(F("Connection"), F("close"));
			request->send(response);

			// Flag the system for reboot which is done in the main loop because it's cleaner
			if (reboot){
				_config->setRebootSystem();
			}
		},
		// Handle file upload (done first and is called multiple times)
		[this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){

			// Set the update status
			_config->setUpdateStatusLocalUpdate();

			// Start of upload
			if (!index){
				Serial.printf("Starting to upload file: %s\n", filename.c_str());
				Update.runAsync(true);
				if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
					Update.printError(Serial);
				}
			}

			// Has the updater encounter an error
			if (!Update.hasError()){
				if(Update.write(data, len) != len){
					Update.printError(Serial);
				}
			}

			// Final call - upload has finished
			if (final){
				if (Update.end(true)){
					Serial.printf("Upload finished: %u bytes\n", index + len);
				}
				else {
					Update.printError(Serial);
				}
			}
		}
	);

	// Server up favicon
	_server.on(PSTR("/favicon.ico"), HTTP_GET, [this](AsyncWebServerRequest *request){
		AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", FAVICON_DATA, ARRAY_LENGTH(FAVICON_DATA));
		request->send(response);
	});

	// ------------------------------------------------------------------
	// REST
	// ------------------------------------------------------------------

	// Get state
	_server.on(PSTR("/rest/state"), HTTP_GET, [this](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), _getJsonSettingIntResponse(_config->getState()));
	});

	// Set state on
	_server.on(PSTR("/rest/state/0"), HTTP_PUT, [this](AsyncWebServerRequest *request){
		request->send(200);
		_config->setState(false);
		_config->updateLight();
		Serial.println(F("REST: state set to off"));
	});

	// Set state off
	_server.on(PSTR("/rest/state/1"), HTTP_PUT, [this](AsyncWebServerRequest *request){
		request->send(200);
		_config->setState(true);
		_config->updateLight();
		Serial.println(F("REST: state set to on"));
	});

	// Get brightness
	_server.on(PSTR("/rest/brightness"), HTTP_GET, [this](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), _getJsonSettingIntResponse(_config->getBrightness()));
	});

	// Retrieve status of any current update operation
	_server.on(PSTR("/rest/updatestatus"), HTTP_GET, [this](AsyncWebServerRequest *request){
		AsyncJsonResponse* response = new AsyncJsonResponse();
		JsonVariant& root = response->getRoot();
		root["status"] = _config->getUpdateStatus();
		root["message"] = _config->getUpdateMessage();
		response->setLength();
		request->send(response);
	});

	// Retrieve all settings as JSON
	_server.on(PSTR("/rest/settings"), HTTP_GET, [this](AsyncWebServerRequest *request){
		AsyncJsonResponse* response = new AsyncJsonResponse();
		JsonVariant& root = response->getRoot();
		root["name"] = _config->getDeviceName();
		root["state"] = _config->getState();
		root["brightness"] = _config->getBrightness();
		root["mode"] = (uint8_t) _config->getMode();
		root["invert"] = _config->getEncoderInverted();
		root["sensitity"] = _config->getEncoderSensitivity();
		root["wificheckinterval"] = _config->getWifiCheckInterval();
		root["automaticupdates"] = _config->getAutomaticUpdates();
		response->setLength();
		request->send(response);
	});

	// Update settings by using PUT with JSON data in body content, e.g.
	// {"name":"aaaaabbbbbcccccdddddeeeeefffffgh","mode":1,"invert":false,"sensitivity":2,"wificheckinterval":11,"automaticupdates":false}
	_server.on(PSTR("/rest/settings"), HTTP_PUT,

		// Handle the http response (called last)	
		[this](AsyncWebServerRequest *request){
			// If the JSON supplied in the body was invalid then the body content callback would have already sent a response
			// We always respond with 200 if we received body content data or not
			request->send(200);
		},
		// Upload callback is not required
		NULL, 
		// Body content callback - called first & the HTTP response is handled in the first callback
		[this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
			static char buffer[140];

			// Clear buffer when first chunk of body is received
			if (!index){
				buffer[0] = 0;
			}

			// Add any received chunks of body to buffer
			strncat(buffer, (const char*) data, len);

			// Have received last packet of data?
			if (index + len == total){

				// Terminate data
				buffer[total] = 0;

				// Output received JSON
				Serial.printf("JSON RECEIVED: %s\n", buffer);

				// Deserialise the JSON
				StaticJsonDocument<200> jdoc;
				DeserializationError error = deserializeJson(jdoc, buffer);
				JsonObject doc = jdoc.as<JsonObject>();

				// Stop if there are any errors with deserialisation
				if (error) {
					Serial.print(F("JSON: deserialization error: "));
					Serial.println(error.c_str());
					request->send(400);
					return;
				}

				// Check that name is a pointer to a string
				if (doc["name"].is<char*>()){
					if (strlen(doc["name"]) > 0){
						char name[_config->getDeviceNameLength()];
						strncpy(name, doc["name"], sizeof(name));
						_config->setDeviceName(name);
					}
				}

				// Check that state is boolean
				if (doc["state"].is<bool>()){
					_config->setState(doc["state"]);
				}

				// Check brightness is uint
				if (doc["brightness"].is<uint8_t>()){
					_config->setBrightness(doc["brightness"]);
				}

				// Check mode is uint
				if (doc["mode"].is<uint8_t>()) {
					_config->setMode(doc["mode"] == 0 ? STANDALONE : WIFI);
				}

				// Check that invert is a bool
				if (doc["invert"].is<bool>()) {
					_config->setEncoderInverted(doc["invert"]);
				}

				// Check that sensitivity is uint
				if (doc["sensitivity"].is<uint8_t>()) {
					if (doc["sensitivity"] <= 6)
						_config->setEncoderSensitivity(doc["sensitivity"]);
				}
				// Check that wifi check interval is uint
				if (doc["wificheckinterval"].is<uint8_t>()) {
					if (doc["wificheckinterval"] <= 60)
						_config->setWifiCheckInterval(doc["wificheckinterval"]);
				}

				// Check automatic updates is a boolean
				if (doc["automaticupdates"].is<bool>()){
					_config->setAutomaticUpdates(doc["automaticupdates"]);
				}

				// Update settings in main thread and save
				_config->updateSettings();

				Serial.println(F("JSON: settings updated!"));
			}
		}
	);

	// Do a restart/reboot
	_server.on(PSTR("/rest/restart"), HTTP_ANY, [this](AsyncWebServerRequest *request){
		request->send(200);
		_config->setRebootSystem();
	});
	_server.on(PSTR("/rest/reboot"), HTTP_ANY, [this](AsyncWebServerRequest *request){
		request->send(200);
		_config->setRebootSystem();
	});

	// REST calls that end in a number or HTTP_OPTIONS requests
	_server.onNotFound([this](AsyncWebServerRequest *request) {
		int16_t value = -1;

		// CORS/Chrome sends HTTP OPTIONS web method and we need to respond with a 200
		// https://stackoverflow.com/questions/33660712/angularjs-post-fails-response-for-preflight-has-invalid-http-status-code-404
		if (request->method() == HTTP_OPTIONS){
			request->send(200);
			return;
		}
	
		// Are we setting parameter via rest
		if (request->url().startsWith(F("/rest/brightness/"))){

			value = _getIntFromEndOfString(request->url());
			
			if (value >= 0 && value <= 255){
				request->send(200);
				Serial.printf("REST: brightness set to: %d\n", value);
				_config->setBrightness(value);
				_config->updateLight();
			}
			else{
				request->send(400);
			}
		} 
		else if (request->url().startsWith("/rest/")){
			// Rest 404 - page not found
			request->send(404);
			Serial.printf("REST 404: %s %s\n", request->methodToString(), request->url().c_str());
		}
		else{
			// 404 - page not found!
			// Create array for our html content
			char code[strlen_P(HTML_ERROR_CODE_404)];
			char description[strlen_P(HTML_ERROR_DESCRIPTION_404)];
			char html[
				strlen_P(HTML_ERROR_PAGE) + 
				sizeof(code) +
				sizeof(description)
			];

			// Copy string from progmem to local variables
			strcpy(code, HTML_ERROR_CODE_404);
			strcpy(description, HTML_ERROR_DESCRIPTION_404);

			// Get HTML page from progmem and populate with our data
			snprintf_P(
				html, sizeof(html),
				HTML_ERROR_PAGE,
				code,
				description
			);

			// Return html
			request->send(200, F("text/html"), html);
			Serial.printf("HTTP 404: %s %s\n", request->methodToString(), request->url().c_str());
		}
	});

	// Allow REST calls from anywhere
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

	// Definies what HTTP methods we can use
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, PUT, GET, OPTIONS");

	// Start asynch web server!
	_server.begin();
	Serial.println(F("WEB SERVER: started"));
}

// Close the server
void AdminWebServer::end(){
	_server.end();
	Serial.println(F("WEB SERVER: stopped"));
}

// Is the system updating?  If so display the updating page
bool AdminWebServer::_systemUpdatingRedirect(AsyncWebServerRequest *request){

	// Check if system is updating
	if (_config->getIsUpdating()){

		// Serve up updating page
		request->send_P(200, F("text/html"), HTML_UPDATING_PAGE);

		return true;
	}

	// We are not updating
	return false;
}