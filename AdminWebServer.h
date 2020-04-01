#ifndef AdminWebServer_h
#define AdminWebServer_h

#include <ESP8266WebServer.h>
#include "Common.h"
#include "Favicon.h"
#include "Config.h"

// HTTP page response
PROGMEM const char HTTP_PAGE_RESPONSE[] = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html\r\n\r\n"
	"<!DOCTYPE HTML>\r\n"
	"<html>\r\n"
	"<h1>Alexa Lamp</h1>\r\n"
	"<table>\r\n"
	"<tr><td>SSID</td><td>%s</td></tr>\r\n" 
	"<tr><td>Uptime</td><td>%s</td></tr>\r\n" 
	"<tr><td>State</td><td>%s</td></tr>\r\n"
	"<tr><td>Brightness</td><td>%d</td></tr>\r\n"
	"<tr><td>Mode</td><td>%s</td></tr>\r\n"
	"<tr><td>Encoder sensitivity</td><td>%d</td></tr>\r\n"
	"<tr><td>Wifi check</td><td>%d mins</td></tr>\r\n"
	"</table>\r\n"
	"</html>\n";

// HTTP favicon response
PROGMEM const char HTTP_FAVICON_RESPONSE[] = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Length: %d\r\n"
	"Content-Type: image/x-icon\r\n\r\n";  //x-icon

// Class to serve a basic web server
class AdminWebServer{
	public:
		AdminWebServer(uint16_t port) : _server(port){}
		void begin(Config* config);
		bool handle();
	private:
		char* getUptime();
		WiFiServer _server;
		Config* _config;
};

#endif