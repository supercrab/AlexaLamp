#ifndef HtmlErrorPage_h
#define HtmlErrorPage_h

// HTTP error code for not found
const char HTML_ERROR_CODE_404[] PROGMEM = "404";

// HTTP error code 404 message
const char HTML_ERROR_DESCRIPTION_404[] PROGMEM = "Page not found!";

const char HTML_ERROR_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Alexa Lamp</title>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<link rel="stylesheet" href='https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css'>
	<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js'></script>
	<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js'></script>
	<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js'></script>
</head>
<body>

<nav class="navbar navbar-expand-sm bg-dark navbar-dark">
	<a class="navbar-brand" href="/">
		<img src='https://raw.githubusercontent.com/supercrab/AlexaLamp/master/assets/light-bulb-icon-32x32.png' width="30" height="30" alt="">
	</a>
	<ul class="navbar-nav">
		<li class="nav-item">
			<a class="nav-link" href="/">Home</a>
		</li>
		<li class="nav-item">
			<a class="nav-link" href="/settings">Settings</a>
		</li>
		<li class="nav-item">
			<a class="nav-link" href="/update">Update</a>
		</li>
		<li class="nav-item">
			<a class="nav-link" href="/about">About</a>
		</li>
	</ul>
</nav>

<div class="jumbotron text-center">
	<h1>%s</h1>
	<div>%s</div> 
</div>

</body>
</html>
)rawliteral";

#endif