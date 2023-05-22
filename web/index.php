<?php

(new DumpHTTPRequestToFile)->execute('./log.txt');

// This PHP files comes from the link below but has been modified
//
// https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html 


// Database of devices and the firmware file for that device
$db = array(
	"alexalamp" => "AlexaLamp.ino"
);

if (check_variable("deviceid") && check_variable("md5")){
	// If we have a device id and md5 parameters then we return JSON
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
	$file = $db[$_GET['deviceid']];
	if(!isset($file)) {
		writeJson(false, false, "The supplied device is not configured for updates");
		exit();
	}

	// Check the bin file exists!
	$path = "./binaries/{$file}.bin";
	if(!file_exists($path)) {
		writeJson(false, false, "The firmware file was not found");
		exit();
	}

	// Check if the md5s are different
	$latestHash = md5_file($path);
	$currentHash = $_GET["md5"];

	if ($currentHash != $latestHash){
		$md5 = $latestHash;
		$message = "There is an update available";
		$update = true;
	}
	else{
		$md5 = $currentHash;
		$message = "No updates are available";
		$update = false;
	}

	// write json
	writeJson(true, $update, $message, $md5);
}

function writeJson($success, $response, $message, $md5){
	$data = [
		'success' => $success, 
		'response' => $response, 
		'message' => $message, 
		'md5' => $md5
	];
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
		echo "Invalid user agent!\n";
		exit();
	}

	// [User-Agent] => ESP8266-http-Update
	// [x-ESP8266-STA-MAC] => 18:FE:AA:AA:AA:AA
	// [x-ESP8266-AP-MAC] => 1A:FE:AA:AA:AA:AA
	// [x-ESP8266-free-space] => 671744
	// [x-ESP8266-sketch-size] => 373940
	// [x-ESP8266-sketch-md5] => a56f8ef78a0bebd812f62067daf1408a
	// [x-ESP8266-chip-size] => 4194304
	// [x-ESP8266-sdk-version] => 1.3.0
	// [x-ESP8266-version] => DOOR-7-g14f53a19
	// [x-ESP8266-mode] => sketch

	// x-ESP8266-Chip-ID		String(ESP.getChipId()));
	// x-ESP8266-STA-MAC		WiFi.macAddress());
	// x-ESP8266-AP-MAC			WiFi.softAPmacAddress());
	// x-ESP8266-free-space 	String(ESP.getFreeSketchSpace()));
	// x-ESP8266-sketch-size	String(ESP.getSketchSize()));
	// x-ESP8266-sketch-md5		String(ESP.getSketchMD5()));
	// x-ESP8266-chip-size		String(ESP.getFlashChipRealSize()));
	// x-ESP8266-sdk-version	ESP.getSdkVersion());
	// x-ESP8266-version		This can be blank, if so then MD5 hash is used to determine update
	// x-ESP8266-device-id      The device id
	if(
		!check_header('HTTP_X_ESP8266_STA_MAC') ||
		!check_header('HTTP_X_ESP8266_AP_MAC') ||
		!check_header('HTTP_X_ESP8266_FREE_SPACE') ||
		!check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
		!check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
		!check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
		!check_header('HTTP_X_ESP8266_SDK_VERSION') ||
		!check_header('HTTP_X_ESP8266_DEVICE_ID')
	) {
		header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
		echo "Missing headers!\n";
		exit();
	}

	// Retrieve the firmware from the database
	$firmware = $db[$_SERVER['HTTP_X_ESP8266_DEVICE_ID']];

	// Was a binary file found?
	if(!isset($firmware)) {
		header($_SERVER["SERVER_PROTOCOL"].' 500 ESP device is not configured for updates', true, 500);
		exit();
	}

	$localBinary = "./binaries/{$firmware}.bin";

	// Check the bin file exists!
	if(!file_exists($localBinary)) {
		header($_SERVER["SERVER_PROTOCOL"].' 500 binary file not found', true, 500);
		exit();
	}

	// Check if version has been set and does not match, if not, check if
	// MD5 hash between local binary and ESP8266 binary do not match if not.
	// then no update has been found.
	if ((!check_header('HTTP_X_ESP8266_SDK_VERSION') && $firmware != $_SERVER['HTTP_X_ESP8266_VERSION'])
		|| $_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($localBinary)) {
		sendFile($localBinary);
	}
	else {
		header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
	}
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
			$data . file_get_contents('php://input') . "\n",
			FILE_APPEND
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