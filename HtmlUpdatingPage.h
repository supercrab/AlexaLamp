#ifndef HtmlUpdatingPage_h
#define HtmlUpdatingPage_h

const char HTML_UPDATING_PAGE[] PROGMEM = R"rawliteral(
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
	<h1>Updating your lamp</h1>
	<div>The firmware on your device is being updated</div> 
</div>

<div class="container">
	<div class="alert alert-warning text-center" id="message-container">
		<strong id="message-title">Firmware is updating</strong>
		<div id="message-text">Please wait while your device is updated...</div>
	</div>
</div>

<script>
	var manualRestartSeconds = 30;
	var manualRestartTimer;
	var ajaxRetrySeconds = 1;
	var ajaxTimeout = (ajaxRetrySeconds * 1000) - 250;

	const status = {
		IDLE: 0,
		NO_UPDATES: 1,
		LOCAL_UPDATE: 2,
		REMOTE_UPDATE: 3,
		FAILED: 4,
		OK: 5
	};

	function manualRestart(){
		$("#message-container").removeClass("alert-warning");
		$("#message-container").addClass("alert-danger");
		$("#message-title").text("Firmware should be finished by now");
		$("#message-text").text("Please power cycle your device to finish the update.  It's also possible that the IP address on the device has changed.");	
	}

	function startTimer(){
		setTimeout(restartCheck, ajaxRetrySeconds * 1000);
	}

	function startManualRestartTimer(){
		if (!manualRestartTimer){
			console.log("Starting manual reset timer...");
			manualRestartTimer = setTimeout(manualRestart, manualRestartSeconds * 1000);
		}
	}

	function clearManualRestartTimer(){
		if (manualRestartTimer){
			console.log("Clearing manual reset timer");
			clearTimeout(manualRestartTimer);
		}
	}

	function restartCheck(){
		$.ajax({
			url: "/rest/updatestatus",
			type: "GET",
			timeout: ajaxTimeout,
			success: function(obj) {
				console.log("Connected OK! " + obj.message);
				clearManualRestartTimer();

				if (obj.status == status.IDLE || obj.status == status.NO_UPDATES){
					$("#message-container").removeClass("alert-warning");
					$("#message-container").addClass("alert-success");

					if (obj.status == status.IDLE){
						$("#message-title").text("Firmware update finished");
						$("#message-text").text("Your device has been successfully updated and is ready to use!");
					}
					else{
						$("#message-title").text("Firmware was not updated");
						$("#message-text").text("Your device is already up to date!");
					}
				}
				else if (obj.status == status.LOCAL_UPDATE || obj.status == status.REMOTE_UPDATE){
					startTimer();
				}
				else if (obj.status == status.FAILED) {
					$("#message-container").removeClass("alert-warning");
					$("#message-container").addClass("alert-danger");
					$("#message-title").text("Firmware update failed");
					$("#message-text").text(obj.message);
				}
			},
			error: function(jqXHR, textStatus, errorThrown) {
				if (textStatus === "timeout"){
					console.log("Failed to connect.  Retrying...");
					startTimer();
				}
				startManualRestartTimer();
			}
		});
	}

	$(document).ready(function(){
		setTimeout(restartCheck, 1000);
	});
</script>
</body>
</html>
)rawliteral";

#endif