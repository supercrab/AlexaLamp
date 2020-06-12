#ifndef HtmlAboutPage_h
#define HtmlAboutPage_h

const char HTML_ABOUT_PAGE[] PROGMEM = R"rawliteral(
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
	<h1>About</h1>
	<div>Information about this system</div> 
</div>

<div class="container text-center">
	<div class="row">
		<div class="col">
			<h2>System Information</h2>
			<dl>
				<dt>Version</dt>
				<dd>%s</dd>
				<dt>Uptime<dt>
				<dd>%s</dd>
				<dt>MD5</dt>
				<dd>%s</dd>
				<dt>Build date</dt>
				<dd>%s %s</dd>
				<dt>Free heap</dt>
				<dd>%d KB</dd>
			</dl>
		</div>
		<div class="col">
			<h2>Credits</h2>
			<img src='https://avatars2.githubusercontent.com/u/1100068?s=200'>
			<p>Created by Mr Mase.</p>
			<p><a class="btn btn-primary" href='https://github.com/supercrab/AlexaLamp' role="button" target="_blank">GitHub</a>&nbsp;<a class="btn btn-primary" href='https://hackaday.io/project/170465-alexa-lamp' role="button" target="_blank">Hackaday</a></p> 
		</div>
	</div>
</div>
</body>
</html>
)rawliteral";

#endif