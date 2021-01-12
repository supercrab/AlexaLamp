#ifndef Common_h
#define Common_h

// ---------------------------------------------------------
// Macros
// ---------------------------------------------------------

// Macro for calculating array length
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

// ---------------------------------------------------------
// Hardware settings (these can be changed to match the hardware)
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

// ---------------------------------------------------------
// Default user settings - these can be changed
// ---------------------------------------------------------

// Initial device name for Alexa purposes 
#define DEFAULT_ALEXA_NAME	"Lamp"

// Initial mode can be WIFI or STANDALONE (WIFI is best)
#define DEFAULT_MODE WIFI

// How often should we check wifi is connected in minutes? (0, 1, 5, 15, 30, 60)
#define DEFAULT_WIFI_CONNECTED_CHECK_MINS 1

// How sensitive the encoder is 
// (1 = slowest, higher means more change)
#define DEFAULT_ENCODER_SENSITIVITY 2

// Default whether we want the encoder direction flipped
#define DEFAULT_ENCODER_INVERTED	false

// Default whether we want automatic updates (check period is defined above)
#define DEFAULT_AUTOMATIC_UPDATES	true

// ---------------------------------------------------------
// Initial lamp settings - these can be changed
// ---------------------------------------------------------

// Initial lamp brightness when turned on for the first time
#define INITIAL_BRIGHTNESS 255

// Initial lamp status (off for safety reasons)
#define INITIAL_STATE	false

// ---------------------------------------------------------
// Software settings - should not need changing
// ---------------------------------------------------------

// Enable MDNS
// The local URL for accessing the web config is http://alexa-lamp.local
// Running MDNS creates extra processing for the MDNS traffic
// If you're not bothered by having a MDNS name for the device then comment out this line
#define ENABLE_MDNS

// Serial baudrate 
#define SERIAL_BAUDRATE 115200

// Admin port for admin web page
#define ADMIN_WEB_PORT 81

// Define how many characters the device's alexa name can be
#define ALEXA_NAME_LENGTH	32

// Network hostname, MDNS name.local
#define HOSTNAME	"alexa-lamp"

// Access point created to configure Wifi settings
#define ACCESS_POINT_NAME "alexa-lamp-connect"

// Number of hours to check for automatic updates
#define AUTO_UPDATE_CHECK_PERIOD_HOURS 24

// ---------------------------------------------------------
// Remote update settings (should not be change)
// ---------------------------------------------------------

// Uodate server
#define HTTP_UPDATE_SERVER "supercrab.co.uk"

// Update path
#define HTTP_UPDATE_PATH "/update/"	

// ---------------------------------------------------------
// Enums
// ---------------------------------------------------------

// Lamp mode
enum operation_t{
	STANDALONE, 
	WIFI
};

// Holds current update status
enum updateStatus_t{
	UPDATE_STATUS_IDLE,				// 0 - not updating
	UPDATE_STATUS_NO_UPDATES,		// 1 - checked for updates but there were nonte
	UPDATE_STATUS_LOCAL_UPDATE,		// 2 - update by an uploaded firmware
	UPDATE_STATUS_REMOTE_UPDATE,	// 3 - update by checking remote server
	UPDATE_STATUS_FAILED,			// 4 - update failed
	UPDATE_STATUS_OK				// 5 - updated worked
};

#endif