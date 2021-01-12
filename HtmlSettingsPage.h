#ifndef HtmlSettingsPage_h
#define HtmlSettingsPage_h

const char HTML_SETTINGS_PAGE[] PROGMEM = R"rawliteral(
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
	<h1>Alexa Lamp</h1>
	<div>Change your lamp's settings</div> 
</div>

<div class="container">  
<form class="needs-validation" novalidate>
<div class="form-group row">
	<label for="name" class="col-sm-3 col-form-label">Name</label>
	<div class="col-sm-9">
		<input type="text" id="name" class="form-control" value="%s" maxlength="%d" placeholder="Alexa device name">
		<div class="invalid-feedback" id="invalid-name">Please provide a valid device name.</div>
		<div class="invalid-feedback" id="name-warning">Note: Alexa will need to discover your device to use the new name.</div>
	</div>
</div>
<div class="form-group row">
	<label for="mode" class="col-sm-3 col-form-label">Mode</label>
	<div class="col-sm-9">%s
		<div class="invalid-feedback" id="mode-warning">Note: this disables Alexa and all web functionality.</div>
	</div>
</div>
<div class="form-group row">
	<label for="invert" class="col-sm-3 col-form-label">Encoder inverted</label>
	<div class="col-sm-9">%s
	</div>
</div>
<div class="form-group row">
	<label for="sensitivity" class="col-sm-3 col-form-label">Encoder sensitivity</label>
	<div class="col-sm-9">%s
	</div>
</div>
<div class="form-group row">
	<label for="wificheckinterval" class="col-sm-3 col-form-label">Wifi check (mins)</label>
	<div class="col-sm-9">%s
	</div>
</div>
<div class="form-group row">
	<label for="automaticupdates" class="col-sm-3 col-form-label">Auto updates</label>
	<div class="col-sm-9">%s
	</div>
</div>
<div class="row">
	<div class="col-sm-12 text-center">
		<button type="button" class="btn btn-primary" id="btn-apply">Apply Changes</button>
	</div>
</div>
</form>
</div>

<div class="modal fade" id="form-results" tabindex="-1" role="dialog" aria-labelledby="form-title" aria-hidden="true">
<div class="modal-dialog modal-dialog-centered" role="document">
<div class="modal-content">
<div class="modal-header">
<h5 class="modal-title" id="form-title">Finishing up</h5>
<button type="button" class="close" data-dismiss="modal" aria-label="Close">
<span aria-hidden="true">&times;</span>
</button>
</div>
<div class="modal-body"></div>
<div class="modal-footer">
<button type="button" class="btn btn-secondary" data-dismiss="modal">Close</button>
</div>
</div>
</div>
</div>

<script>
function showPopup(title, body){
	$("#form-results").modal("show");
	$(".modal-title").text(title);
	$(".modal-body").text(body);
	setTimeout(
		function(){
			$("#form-results").fadeOut("slow", function(){$("#form-results").modal("hide"); });
		},
		5000
	);
}

function save(){
	if ($("#name").val() == ""){
		$("#invalid-name").fadeIn("slow");
		return;
	}
	else
		$("#invalid-name").hide();

	var url = "/rest/settings";
	var settings = {
		name: $("#name").val(),
		mode: parseInt($("#mode").val()),
		invert: parseInt($("#invert").val()) == 1 ? true : false,
		sensitivity: parseInt($("#sensitivity").val()),
		wificheckinterval: parseInt($("#wificheckinterval").val()),
		automaticupdates: parseInt($("#automaticupdates").val()) == 1 ? true : false
	};

	$.ajax({
		url: url,
		type: "PUT",
		contentType: "application/json",
		datatype: "text",
		data: JSON.stringify(settings),
		success: function() {
			showPopup("Success", "Your changes were applied successfully!");
		},
		error: function(jq, status, error){
			showPopup("There was a problem", "Your changes were not saved.  Web call error: " + error);
		}
	});
}

$(document).ready(function(){
	$("#btn-apply").click(save);
	$("#mode").change(function(){
		$(this).val() == 0 ? $("#mode-warning").fadeIn("slow") : $("#mode-warning").hide();	
	});
	$("#name").change(function(){
		$(this).val() != "" && $(this).val() != "%s" ? $("#name-warning").fadeIn("slow") : $("#name-warning").hide();	
	});
});
</script>
</body>
</html>
)rawliteral";

#endif