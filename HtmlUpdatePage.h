#ifndef HtmlUpdatePage_h
#define HtmlUpdatePage_h

// Update form with: https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
const char HTML_UPDATE_PAGE[] PROGMEM = R"rawliteral(
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
	<h1>Update Firmware</h1>
	<div>Check for updates and upgrade your firmware</div> 
</div>

<div class="container">
	<h2>Remote Update</h2> 
	<div>
		<p id="update-info" class="alert alert-primary">Checking server for update...</p>
	</div>

	<div class="p-3 mb-2 bg-warning text-dark"> 
		<h2>Custom Firmware</h2>
		<div class="alert alert-warning" role="alert">Only use this feature if you have a valid firmware file!  If you are not sure use the remote update check above.</div>

		<form method="POST" action="#" enctype="multipart/form-data" id="upload-form">
			<div class="input-group mb-3">
				<div class="custom-file">
					<input type="file" class="custom-file-input" name="update">
					<label class="custom-file-label" for="update">Firmware file to upload</label>
				</div>
				<div class="input-group-append">
					<input type="submit" value="Upload" class="btn btn-primary" id="submit-button" disabled>
				</div>
			</div>
		</form>

		<p id="progress-message" class="text-center"></p>
		<div class="progress">
			<div class="progress-bar" role="progressbar" aria-valuenow="0" aria-valuemin="0" aria-valuemax="100"></div>
		</div>
	</div>
</div>

<div class="modal fade" id="wait-popup" tabindex="-1" role="dialog" aria-labelledby="modal-title" aria-hidden="true">
	<div class="modal-dialog modal-dialog-centered" role="document">
		<div class="modal-content">
			<div class="modal-header">
				<h5 class="modal-title" id="modal-title">Finishing up</h5>
				<button type="button" class="close" data-dismiss="modal" aria-label="Close">
					<span aria-hidden="true">&times;</span>
				</button>
			</div>
			<div class="modal-body">Please wait...</div>
			<div class="modal-footer">
				<button type="button" class="btn btn-secondary" data-dismiss="modal">Close</button>
			</div>
		</div>
	</div>
</div>

<script>
	function displayResult(html){
		$("#update-info").html(html);
	}

	function displayError(text){
		$("#update-info").addClass("alert alert-danger");
		$("#update-info").text(text);
	}

	$(".custom-file-input").change(function(){
		$("#submit-button").removeAttr("disabled");
	});

	$(document).ready(function(){
		$.getJSON(
			'http://supercrab.co.uk/update/?mac=%s&md5=%s',
			function(data) {
				if (data){
					if (data.success === true){
						if (data.response === true){
							displayResult("<a href='/updatenow' class='btn btn-success'>Click here to upgrade!</a>");
						}
						else{
							$("#update-info").addClass("alert alert-success");
							displayResult("Your firmware is currently up to date!");
						}
					}
					else{
						displayError(data.message);
					}
				}
				else{
					displayError("Remote server error!");
				}
			}
		)
		.fail(function(){
			displayError("Could not access remote server to check for updates!");
		});
	});

	$("form").submit(function(e){
		e.preventDefault();
		var form = $("#upload-form")[0];
		var data = new FormData(form);

		$.ajax({
			url: "/update", 
			type: "POST", 
			data: data, 
			contentType: false, 
			processData: false, 
			xhr: function() {
				var xhr = new window.XMLHttpRequest();
				xhr.upload.addEventListener("progress", function(evt) {
					if (evt.lengthComputable) {
						var per = evt.loaded / evt.total;
						$("#progress-message").html("Progress: " + Math.round(per*100) + "%");
						$(".progress-bar").css("width", Math.round(per*100) + "%");
						if (per == 1) $('#wait-popup').modal('show');
					}
				}, false);
			return xhr;
			}, 
			success: function(d, s) {
				console.log("File uploaded!");
				
				$("body").html(d);
			},
			error: function(a, b, c) {
			}
		});
	});
</script>
</body>
</html>
)rawliteral";

#endif