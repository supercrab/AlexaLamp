#ifndef HtmlFormElements_h
#define HtmlFormElements_h

// Html for mode form element
const char HTML_MODE_INPUT[] PROGMEM = R"rawliteral(
<select id="mode" class="form-control">
	<option %s value="0">Standalone</option>
	<option %s value="1">Wifi</option>
</select>)rawliteral";

// Html for invert encoder form element
const char HTML_INVERT_INPUT[] PROGMEM = R"rawliteral(
<select id="invert" class="form-control">
	<option %s value="0">No</option>
	<option %s value="1">Yes</option>
</select>)rawliteral";

// Html for encoder sensitivty form element
const char HTML_SENSITIVITY_INPUT[] PROGMEM = R"rawliteral(
<select id="sensitivity" class="form-control">
	<option %s value="1">1</option>
	<option %s value="2">2</option>
	<option %s value="3">3</option>
	<option %s value="4">4</option>
	<option %s value="5">5</option>
	<option %s value="6">6</option>
</select>)rawliteral";

// Html for wifi check interval form element
const char HTML_WIFI_CHECK_INTERVAL_INPUT[] PROGMEM = R"rawliteral(
<select id="wificheckinterval" class="form-control">
	<option %s value="0">Off</option>
	<option %s value="1">1</option>
	<option %s value="5">5</option>
	<option %s value="15">15</option>
	<option %s value="30">30</option>
	<option %s value="60">60</option>
</select>)rawliteral";

// Html for automatic updates form element
const char HTML_AUTOMATIC_UPDATES_INPUT[] PROGMEM = R"rawliteral(
<select id="automaticupdates" class="form-control">
	<option %s value="0">No</option>
	<option %s value="1">Yes</option>
</select>)rawliteral";

#endif