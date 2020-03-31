#include "AdminWebServer.h"

// Start the webserver
void AdminWebServer::begin(){
	_server.begin();
}

// Pass application config into web server
void AdminWebServer::setConfig(Config* config){
	_config = config;
}

// Retrieve uptime formatted
char* AdminWebServer::getUptime(){
	static char buffer[64];
	uint32_t sec = millis() / 1000;
	uint32_t min = sec / 60;
	uint16_t hr = min / 60;
	uint16_t day = hr / 24;
	sprintf(buffer, "%d days, %d hours, %d mins, %d seconds", day, hr, min % 60, sec % 60);
	return buffer;
}

// Handle any incoming web client requests
bool AdminWebServer::handle(){

	// See if there's a client reques to process
	WiFiClient client = _server.available();
	
	// Do nothing if there's no client
	if (!client) return false;

	// Wait 250ms for the client to send a request
	unsigned long timeout = millis() + 250; 
	while (!client.available() && millis() < timeout){
		delay(1);
	}
	// Client never responded in time 
	if (!client.available()) return false;

	// Read the first line of the request
	String request = client.readStringUntil('\r');
	Serial.printf("HTTP Request: %s ", request.c_str());
	client.flush();

	// Admin page requested
	if (request.startsWith("GET / ")){
		// Generate admin page HTTP page
		char response[strlen_P(HTTP_PAGE_RESPONSE) + 64]; // extra 64 characters for text we insert
		snprintf_P(
			response, sizeof(response),
			HTTP_PAGE_RESPONSE,
			WiFi.SSID().c_str(),
			getUptime(),
			_config->state ? "On" : "Off",
			_config->brightness,
			_config->wifi ? "Wifi" : "Standalone",
			_config->encoderSensitivity,
			_config->wifiCheckInterval
		);
		client.print(response);
		Serial.println(" Served page!");
		return true;
	}

	// Favicon requested
	else if (request.startsWith("GET /favicon.ico")){
		// Generate HTTP response
		char response[strlen_P(HTTP_FAVICON_RESPONSE) + 8]; // extra 8 chars for favicon length in bytes
		const int faviconLength = ARRAY_LENGTH(FAVICON_DATA);
		snprintf_P(
			response, sizeof(response), 
			HTTP_FAVICON_RESPONSE, // HTTP image response
			faviconLength // HTTP Content-Length
		);
		// Write HTTP response
		client.print(response);
		// Write image char array 
		client.write_P(FAVICON_DATA, faviconLength);
		Serial.println(" Served favicon!");
		return true;
	}

	// Did not receive a valid request
	Serial.println(" Ignored!");
	return false;
}