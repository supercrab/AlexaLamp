#ifndef HtmlHomePage_h
#define HtmlHomePage_h

const char HTML_HOME_PAGE[] PROGMEM = R"rawliteral(
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
	<style>
		.alert{
			display: none;
		}
	</style>
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
	<h1>Alexa Lamp</h1>
	<div>Here you can control your lamp</div> 
</div>

<div class="container">  
	<form>
		<div class="form-group row">
			<label for="state-on" class="col-sm-2 col-form-label">Status</label>
			<div class="col-sm-10">
				<button type="button" class="btn btn-outline-primary" value="1" id="state-on">Lamp On</button>
				<button type="button" class="btn btn-outline-primary" value="0" id="state-off">Lamp Off</button>
			</div>
		</div>

		<div class="form-group row">
			<label for="brightness" class="col-sm-2 col-form-label">Brightness</label>
			<div class="col-sm-10">
				<input type="range" class="custom-range" min="0" max="255" value="%d" id="brightness">
			</div>
		</div>
	</form>
	<div class="alert alert-success" role="alert" id="alert-success">
		<strong>Success!</strong>
		<div>Your change was applied successfully</div>
	</div>
	<div class="alert alert-danger" role="alert" id="alert-error">
		<strong>There was a problem!</strong>
		<div>Your change was not saved.  Web call error: <span id="alert-error-message"></span></div>
	</div>
</div>
<script>
	var t = 0; 

	function populateState(){
		$.getJSON(
			"/rest/state",
			function(data) {
				if (data.value === 0){
					$("#state-off").removeClass("btn-outline-primary");
					$("#state-off").addClass("btn-primary");
					$("#state-on").removeClass("btn-primary");
					$("#state-on").addClass("btn-outline-primary");
				}
				if (data.value === 1){
					$("#state-on").removeClass("btn-outline-primary");
					$("#state-on").addClass("btn-primary");
					$("#state-off").removeClass("btn-primary");
					$("#state-off").addClass("btn-outline-primary");
				}
			}
		);
	}

	function alertSuccess(){
		$("#alert-error").hide();
		$("#alert-success").fadeIn("slow");
		fadeOut("#alert-success");
	}

	function alertError(error){
		$("#alert-success").hide();
		$("#alert-error-message").text(error);
		$("#alert-error").fadeIn("slow");
		fadeOut("#alert-error");
	}
	
	function fadeOut(element){
		if (t) clearTimeout(t);
		t = setTimeout(function(){ $(element).fadeOut("slow"); }, 2000);
	}

	function updateSetting(){
		var stateChange = this.id.startsWith("state");
		var parameter = stateChange ? "state" : this.id;
		var value = this.value;
		var url = "/rest/" + parameter + "/" + value;

		$.ajax({
			url: url,
			type: 'PUT',
			success: function() {
				alertSuccess();
				if (stateChange) populateState();
			},
			error: function(jq, status, error){
				if (!error) error = 'could not access lamp!';
				alertError(error);
			}
		});
	}

	$(document).ready(function(){
		$("button").click(updateSetting);
		$("#brightness").change(updateSetting);
		populateState();
	});
</script>
</body>
</html>
)rawliteral";

#endif