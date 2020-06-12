<?php

// List of MAC addresses and what bin file is applicable
// this allows bin filename to be changed BUT only if HTTP_X_ESP8266_VERSION header is set
$db = array(
	"AA:BA:BE:AA:FA:CE" => "AlexaLamp.ino"
);

//(new DumpHTTPRequestToFile)->execute('./log.txt');

if (check_variable("mac") && check_variable("md5")){
	// If we have mac and md5 parameters then we return JSON
	ESPhttpUpdateCheck();
}
else{
	// Do the standard ESP http update
	ESPhttpUpdate();
}
	
// Simple HTTPcheck
function ESPhttpUpdateCheck(){
	global $db;

	// Was a binary file found?
	$file = $db[$_GET['mac']];
	if(!isset($file)) {
		writeJson(false, false, "The supplied MAC is not configured for updates");
		exit();
	}

	// Check the bin file exists!
	$path = "./binaries/{$file}.bin";
	if(!file_exists($path)) {
		writeJson(false, false, "The firmware file was not found");
		exit();
	}

	// Check if the md5s are different
	$update = $_GET["md5"] != md5_file($path);

	// Generate user message depending on if there is an update available
	if ($update)
		$message = "There is an update available";
	else
		$message = "No updates are available";

	// write json
	writeJson(true, $update, $message);
}

function writeJson($success, $response, $message = ""){
	$data = ['success' => $success, 'response' => $response, 'message' => $message];
	header('Access-Control-Allow-Origin: *');
	header('Content-type: application/json');
	print(json_encode($data));
}

function check_variable($name){
	return isset($_GET[$name]) && !empty($_GET[$name]);
}

function check_header($name, $value = false) {
	if(!isset($_SERVER[$name])) {
		return false;
	}
	if($value && $_SERVER[$name] != $value) {
		return false;
	}
	return true;
}

function sendFile($path) {
	header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
	header('Content-Type: application/octet-stream', true);
	header('Content-Disposition: attachment; filename='.basename($path));
	header('Content-Length: '.filesize($path), true);
	header('x-MD5: '.md5_file($path), true);
	readfile($path);
}

// The OG http update
function ESPhttpUpdate(){
	global $db;

	header('Content-type: text/plain; charset=utf8', true);

	if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
		header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
		echo "only for ESP8266 updater!\n";
		exit();
	}

	// x-ESP8266-Chip-ID		String(ESP.getChipId()));
	// x-ESP8266-STA-MAC		WiFi.macAddress());
	// x-ESP8266-AP-MAC			WiFi.softAPmacAddress());
	// x-ESP8266-free-space 	String(ESP.getFreeSketchSpace()));
	// x-ESP8266-sketch-size	String(ESP.getSketchSize()));
	// x-ESP8266-sketch-md5		String(ESP.getSketchMD5()));
	// x-ESP8266-chip-size		String(ESP.getFlashChipRealSize()));
	// x-ESP8266-sdk-version	ESP.getSdkVersion());
	// x-ESP8266-version		This can be blank, if so then MD5 hash is used to determine update
	if(
		!check_header('HTTP_X_ESP8266_STA_MAC') ||
		!check_header('HTTP_X_ESP8266_AP_MAC') ||
		!check_header('HTTP_X_ESP8266_FREE_SPACE') ||
		!check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
		!check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
		!check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
		!check_header('HTTP_X_ESP8266_SDK_VERSION')
	) {
		header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
		echo "only for ESP8266 updater! (header)\n";
		exit();
	}

	// Was a binary file found?
	if(!isset($db[$_SERVER['HTTP_X_ESP8266_STA_MAC']])) {
		header($_SERVER["SERVER_PROTOCOL"].' 500 ESP MAC not configured for updates', true, 500);
		exit();
	}

	$localBinary = "./binaries/".$db[$_SERVER['HTTP_X_ESP8266_STA_MAC']].".bin";

	// Check the bin file exists!
	if(!file_exists($localBinary)) {
		header($_SERVER["SERVER_PROTOCOL"].' 500 binary file not found', true, 500);
		exit();
	}

	// Check if version has been set and does not match, if not, check if
	// MD5 hash between local binary and ESP8266 binary do not match if not.
	// then no update has been found.
	if ((!check_header('HTTP_X_ESP8266_SDK_VERSION') && $db[$_SERVER['HTTP_X_ESP8266_STA_MAC']] != $_SERVER['HTTP_X_ESP8266_VERSION'])
		|| $_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($localBinary)) {
		sendFile($localBinary);
		exit();
	}
	else {
		header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
		exit();
	}

	header($_SERVER["SERVER_PROTOCOL"].' 500 ESP MAC not configured for updates', true, 500);
}

// Logging class
class DumpHTTPRequestToFile {
	public function execute($targetFile) {
		$data = sprintf(
			"%s %s %s\n\nHTTP headers:\n",
			$_SERVER['REQUEST_METHOD'],
			$_SERVER['REQUEST_URI'],
			$_SERVER['SERVER_PROTOCOL']
		);
		foreach ($this->getHeaderList() as $name => $value) {
			$data .= $name . ': ' . $value . "\n";
		}
		$data .= "\nRequest body:\n";
		file_put_contents(
			$targetFile,
			$data . file_get_contents('php://input') . "\n"
		);
	}
	private function getHeaderList() {
		$headerList = [];
		foreach ($_SERVER as $name => $value) {
			if (preg_match('/^HTTP_/',$name)) {
				// convert HTTP_HEADER_NAME to Header-Name
				$name = strtr(substr($name,5),'_',' ');
				$name = ucwords(strtolower($name));
				$name = strtr($name,' ','-');

				// add to list
				$headerList[$name] = $value;
			}
		}
		return $headerList;
	}
}

?>