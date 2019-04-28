// --- Data exchange functions ---
var serverSetting = false;
var ws;

ws = new WebSocket(url("/ws"));

// subscribe to visibility change events
document.addEventListener('visibilitychange', function() {
	// fires when app transitions from prerender, user returns to the app / tab.
	if (document.visibilityState == 'visible') { 
		if (!ws || !(ws.readyState == WebSocket.OPEN || ws.readyState == WebSocket.CONNECTING)) {
			ws = new WebSocket(url("/ws"));
		}
	}
});

ws.onopen = function(event) {
	try {
		pageRefresh($("#pills-tab li .active").data("id"));
	} catch (e) {			
		(console.error || console.log).call(console, e.stack || e);
	}
}

ws.onmessage = function(event) {
  var msg = JSON.parse(event.data);
  
  switch(msg.type) {
	case "sv.update":
	case "sv.init.wifi":		// Received initialization object
	case "sv.init.settings":	// Received initialization object
	case "sv.init.time":	// Received initialization object
	case "sv.init.leds":		// Received initialization object	   
		updateElements(msg.value);
		break;						
	case "sv.status":			// Received initialization object	
		updateStatus(msg.value);
		break;
  }
};

function url(s) {
	var l = window.location;
	return ((l.protocol === "https:") ? "wss://" : "ws://") + l.host + s;
}

function elementChange(element) {
	if (serverSetting) {
		return;
	}
	var value = getElementValue(element);
	
	var msg = '5:' + element.id + ':'+ value;
	sendMsg(msg);
}

function getElementValue(element) {
	element = $(element);
	var elType = element.attr('type');
	if (elType == "checkbox") {
		return element.prop("checked");
	} else {
		return element.val();
	}
}

function pageRefresh(activePage){
	switch (activePage) {
		case 'wifi': 
			sendMsg('1:'); 
			break;
		case 'settings': 
			sendMsg('2:'); 			
			break;
		case 'leds': 
			sendMsg('3:'); 	
			break;	
		case 'time': 
			sendMsg('4:'); 	
			break;				
	}		
} ;

function sendMsg(msg) {
	console.log(msg);
	try {
		ws.send(msg);
	} catch (e) {
		console.log(e.stack || e);
	}
}

function updateElements(values) {
	serverSetting = true;
	
	var displayOn = values["display_on"];
	var displayOff = values["display_off"];
	
	if (typeof displayOn != 'undefined' && typeof displayOff != 'undefined') {
		$("#aut_display_on").slider("setValue",[displayOn,displayOff]);
		delete values["display_on"];
		delete values["display_off"];
	}
	
	Object.keys(values).forEach(function(key, index) {
		var element = $('#' + key);
		var elType = element.attr('type');
		
		//console.log(elType); 
		
		if (elType == "checkbox") {
			if(this[key] == 1){
				$(element).bootstrapToggle('on');
			}else{
				$(element).bootstrapToggle('off');
			}
		}else if (elType == "selectmenu") {
			element.val(this[key]);
			//element.selectmenu("refresh");
		}else {
			if(key == "colorpickerfield_led_strip"){
				pickr.setColor("#"+this[key]);
			}else if(key == "led_strip_brightness"){
				$("#led_strip_brightness").slider("setValue",this[key]);
				$("#txt_led_strip_brightness").val(this[key]);
			}else if(key == "led_strip_speed"){
				$("#led_strip_speed").slider("setValue",this[key]);
				$("#txt_led_strip_speed").val(this[key]);
			}else {
				element.val(this[key]);
			}
			if (elType == "number") {
				element.slider("refresh");					
			}
		}
	}, values);
	serverSetting = false;

	// Patterns - Display mode	
	if($("#patterns option:selected").val()!="0" && ($("#patterns").data("loaded")+"") == "false"){
		if($("#patterns option:selected").data("tile") == true){
			$("#display_mode").prop("checked",true).trigger("change");
		}else{
			$("#display_mode").removeProp("checked").trigger("change");
		}
		//$("#display_mode option[value="+$("#patterns option:selected").data("tile")+"]").attr('selected','selected').trigger("change");
		$("#patterns,#fixed_patterns").attr("data-loaded","true");
	}else{
		if($("#fixed_patterns option:selected").val()!="0" && ($("#fixed_patterns").data("loaded")+"") == "false"){
			if($("#patterns option:selected").data("tile") == true){
				$("#display_mode").prop("checked",true).trigger("change");
			}else{
				$("#display_mode").removeProp("checked").trigger("change");
			}
			//$("#display_mode option[value="+$("#fixed_patterns option:selected").data("tile")+"]").attr('selected','selected').trigger("change");
			$("#patterns,#fixed_patterns").attr("data-loaded","true");
		}else{
			if($("#patterns option:selected").data("tile") == true){
				$("#display_mode").prop("checked",true).trigger("change");
			}else{
				$("#display_mode").removeProp("checked").trigger("change");
			}
			//$("#display_mode option[value=false]").attr('selected','selected').trigger("change");
			$("#patterns,#fixed_patterns").attr("data-loaded","true");
		}
	}
}
// ------------------------------------------------------------------------

// ------ Data ------
var wifiData = {
	type: "sv.init.wifi",
	value: {
		ssid: "ssid",
		password: "password",
		hostname: "hostname"
	}
};

var statusData = {
	type: "sv.status",
	value: ""
};

var settingsData = {
	type: "sv.init.settings",
	value: {				
		led_strip_nr_of_leds_corner: 1,						
		led_strip_nr_of_leds: 3, 					
		led_strip_nr_sides_tile: "", 
		mqtt_host_name: "",
		mqtt_port: "",
		mqtt_user_name: "",
		mqtt_user_password: "",
		mqtt_client_id: ""
	}
};
	
var ledsData = {
	type: "sv.init.leds",
	value: {
		pattern_color: 0,
		patterns: 0,
		fixed_patterns: 0,
		effects: 0,
		display_mode: false,
		aut_display_enabled: true,
		display_on: 5,
		display_off: 22,
		backlight: true,
		pir_sensor_endabled: false,
		pir_sensor_display_on_time: 0,
		allow_led_params_mod: false,
		colorpickerfield_led_strip: "00ff00",												
		led_strip_brightness: 15,
		led_strip_speed: 20
	}
};

var timeData = {
	type: "sv.init.time",
	value: {
		tz_dst_rule_tz: "A",
		tz_dst_rule_week: 1,
		tz_dst_rule_dow: 1,
		tz_dst_rule_month: 1,
		tz_dst_rule_hour: 0, 
		tz_dst_rule_offset: 0,
		tz_std_rule_tz: "A",
		tz_std_rule_week: 1,
		tz_std_rule_dow: 1,
		tz_std_rule_month: 1,
		tz_std_rule_hour: 0, 
		tz_std_rule_offset: 0,		
		ntp_server: "",
		ntp_date_time: ""
	}
};

// ------------------------------------------------------------------------

// --- Status field ---
function updateStatus(msg) {
	$("#status").html(msg.replace(/\r\n/g, '<br />'))		
}
// ------------------------------------------------------------------------
// --- Elements functions ---
$(function() {
	// --- Sliders ---
	// Automatic display On
	$('.chkToggle').bootstrapToggle();
	var sldAutDisplayOn = $("#aut_display_on").slider({
		ticks: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23],
		ticks_labels: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23],
		ticks_snap_bounds: 30
	}).on('slideStop', function(){
		if (serverSetting) {
			return;
		}

		var values = sldAutDisplayOn.getValue();
		var msg = '5:display_on:'+ values[0];
		sendMsg(msg);
		msg = '5:display_off:'+ values[1];
		sendMsg(msg);
	}).data('slider');

	// LED strip parameters - Brightness, Speed
	$("input[type='number']").change(function(){
		if($(this).val().trim() == ""){
			$(this).val(0);
		}
		if(parseInt($(this).val().trim()) < parseInt($(this).attr("min"))){
			$(this).val($(this).attr("min"));
		}
		if(parseInt($(this).val().trim()) > parseInt($(this).attr("max"))){
			$(this).val($(this).attr("max"));
		}
	});

	// Display mode
	$("#display_mode").change(function(){
		$("#patterns option,#fixed_patterns option").each(function(){
			if($(this).data("tile") != $("#display_mode").is(":checked")){
				$(this).addClass("d-none");
			}else{
				$(this).removeClass("d-none");
			}
		});
	});

	// LED strip Brightness
	var sldBrightness = $("#led_strip_brightness").slider({});
	$("#led_strip_brightness").on("slide", function(slideEvt) {
		$("#txt_led_strip_brightness").val(slideEvt.value);
	}).on('slideStop', function(){
		elementChange(this);
	}).data('slider');
	$("#txt_led_strip_brightness").change(function(){
		sldBrightness.slider('setValue',$(this).val());
		$("#led_strip_brightness").trigger("slideStop");
	});

	// LED strip Speed
	var sldSpeed = $("#led_strip_speed").slider({});
	$("#led_strip_speed").on("slide", function(slideEvt) {
		$("#txt_led_strip_speed").val(slideEvt.value);
	}).on('slideStop', function(){
		elementChange(this);
	}).data('slider');;
	$("#txt_led_strip_speed").change(function(){
		sldSpeed.slider('setValue',$(this).val());
		$("#led_strip_speed").trigger("slideStop");
	});

	// --- Select menu ---
	// Tile patterns
	$(".led-pattern").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);
		
		// Clear Pattern selector
		if($(this).attr("id") == "patterns"){
			$("#fixed_patterns").val(0);
		}

		if($(this).attr("id") == "fixed_patterns"){
			$("#patterns").val(0);
			$("#pattern_color").val(0);
		}
	}); 
	
	// PIR sensor LEDs Time ON
	$(".pir-sensor-display-on-time").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);
	}); 	
	
	// Daylight Timezone
	$(".tz-dst-rule-tz").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 	

	// Daylight Week
	$(".tz-dst-rule-week").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 
	
	// Daylight Day of week
	$(".tz-dst-rule-dow").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 	

	// Daylight month
	$(".tz-dst-rule-month").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 
	
	// Daylight hour
	$(".tz-dst-rule-hour").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 

	// Daylight offset
	$(".tz-dst-rule-offset").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 	
	
	// Standard Timezone
	$(".tz-std-rule-tz").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 	

	// Standard Week
	$(".tz-std-rule-week").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 
	
	// Standard Day of week
	$(".tz-std-rule-dow").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 	

	// Standard month
	$(".tz-std-rule-month").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 
	
	// Standard hour
	$(".tz-std-rule-hour").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	}); 

	// Standard offset
	$(".tz-std-rule-offset").on("change",function(){
		var value = $(this).val();
		var msg = '5:'+$(this).data("type")+':'+ value;
		sendMsg(msg);	
	});	
	
	// --- Buttons ---
	// Set LED Properties button
	$('#btnSetLedsSettingsProp').click(function(){
		var msg = '5:saveDefaultValues:1';
		sendMsg(msg);
	});

	// Set LED Settings
	$('#btnSetLedsSettings').click(function(){
		$(document.body).css({'cursor' : 'wait'});
		
		var value = $('#ntp_server').val();
		var msg = '5:setNtpServer:' + value;
	
		sendMsg(msg);
		
		value = $('#led_strip_nr_of_leds_corner').val();
		msg = '5:setNrOfLedsCorner:' + value;
		sendMsg(msg);

		value = $('#led_strip_nr_of_leds').val();
		msg = '5:setNrOfLeds:' + value;
		sendMsg(msg);

		value = $('#led_strip_nr_sides_tile').val();
		// Trim unnecessary data
		var charPosition = value.indexOf("_");
		if (charPosition > 0)
			value = value.slice(value, charPosition - 1)
		msg = '5:setNrOfSidesPerTile:' + value;
		sendMsg(msg);
		
		// Call save values
		msg = '5:saveLedsSettings:1';
		sendMsg(msg);
	
		// Call restart
		setTimeout(function() {                           
			msg = '5:restart:1';
			sendMsg(msg);
		}, 2000)
		
		// Reload page after 10s
		setTimeout(function() {
			location.reload();
		}, 12000)
	}); 
	
	// Set MQTT parameters
	$('#btnSetMqtt').click(function(){
		$(document.body).css({'cursor' : 'wait'});
		
		var value = $('#mqtt_host_name').val();
		var msg = '5:setMqttHostName:' + value;
	
		sendMsg(msg);

		value = $('#mqtt_port').val();
		msg = '5:setMqttPort:' + value;
		sendMsg(msg);

		value = $('#mqtt_user_name').val();
		msg = '5:setMqttUserName:' + value;
		sendMsg(msg);
	
		value = $('#mqtt_user_password').val();
		msg = '5:setMqttUserPassword:' + value;
		sendMsg(msg);						

		value = $('#mqtt_client_id').val();
		msg = '5:setMqttClient:' + value;
		sendMsg(msg);

		// Call save values
		msg = '5:saveMqttSettings:1';
		sendMsg(msg);
		
		// Call restart
		setTimeout(function() {                            
			msg = '5:restart:1';
			sendMsg(msg);
		}, 2000)
		
		// Reload page after 10s
		setTimeout(function() {
			location.reload();
		}, 12000)
	}); 

	// Set Time Properties button
	$('#btnSetTime').click(function(){
		$(document.body).css({'cursor' : 'wait'});

		var value = $('#ntp_server').val();
		var msg = '5:setNtpServer:' + value;
	
		sendMsg(msg);
		
		var msg = '5:saveTimeSettings:1';
		sendMsg(msg);
		
		// Call restart
		setTimeout(function() {                            
			msg = '5:restart:1';
			sendMsg(msg);
		}, 2000)		
		
		// Reload page after 10s
		setTimeout(function() {
			location.reload();
		}, 12000)
	});		
	
	// --- Input masks ---
	// Nr. of sides per Tile
	document.getElementById("led_strip_nr_sides_tile").onkeypress = validate_int;
	document.getElementById("led_strip_nr_sides_tile").onkeyup = nr_sides_tile_mask;

	// Load LEDs data
	pageRefresh('leds');

	$('a[data-toggle="pill"]').on('shown.bs.tab', function (e) {
		pageRefresh($(e.target).data("id"));
	});
});

// --- Color picker ---
const pickr = Pickr.create({
		el: '#colorpickerfield_led_strip',
		useAsButton: false,
		components: {
			// Main components
			preview: false,
			opacity: false,
			hue: true,

			// Input / output Options
			interaction: {
				hex: false,
				rgba: false,
				hsla: false,
				hsva: false,
				cmyk: false,
				input: true,
				clear: true,
				save: true
			}
		}
	});

var initPickrDone = 0;
	
pickr.on('init', (...args) => {
		//console.log('init', args);
	}).on('save', (hsva, instance, ...args) => {
		if (initPickrDone == 0 && hsva.toHEX().toString() == '#FFFFFF') {		
		
		}
		else {
			var msg = '5:colorpickerfield_led_strip:'+ hsva.toHEX().toString().replace("#","");
			sendMsg(msg);			
			initPickrDone = 1;
		}
		//console.log('save', args);
	}).on('change', (...args) => {
		//console.log('change', args);
	}).on('swatchselect', (...args) => {
		//console.log('swatchselect', args);
	});	

// --- Input mask ---
// Nr. of sides per Tile
/**
 * charCode [48,57] 	Numbers 0 to 9
 * keyCode 46  			"delete"
 * keyCode 9  			"tab"
 * keyCode 13  			"enter"
 * keyCode 116 			"F5"
 * keyCode 8  			"backscape"
 * keyCode 37,38,39,40	Arrows
 * keyCode 10			(LF)
 */
function validate_int(inputValue) {
	if ((inputValue.charCode >= 48 && inputValue.charCode <= 57) || inputValue.keyCode == 9 || inputValue.keyCode == 10 || inputValue.keyCode == 13 || inputValue.keyCode == 8 || inputValue.keyCode == 116 || inputValue.keyCode == 46 || (inputValue.keyCode <= 40 && inputValue.keyCode >= 37)) {
	ret = true;
	} else {
	ret = false;
	}
	return ret;
}

function nr_sides_tile_mask() {
	var valueMask = "_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_";
	var valueInputID = document.getElementById("led_strip_nr_sides_tile");
	var valueText = "";
	var valueNumbers = [];
	var valueOutput = ""
	var lastPosition = 1;
	valueText = valueInputID.value;

	// Get numbers
	for (var i = 0; i < valueText.length; i++) {
		if (!isNaN(valueText.charAt(i)) && valueText.charAt(i) != " ") {
		  valueNumbers.push(valueText.charAt(i));
		}
	}

	// Write over mask
	for (var j = 0; j < valueMask.length; j++) {
		if (valueMask.charAt(j) == "_") { //replace "_" with a number 
		  if (valueNumbers.length == 0)
			valueOutput = valueOutput + valueMask.charAt(j);
		  else {
			valueOutput = valueOutput + valueNumbers.shift();
			lastPosition = j + 1; //set current position
		  }
		} else {
		  valueOutput = valueOutput + valueMask.charAt(j);
		}
	}
	document.getElementById("led_strip_nr_sides_tile").value = valueOutput;
	document.getElementById("led_strip_nr_sides_tile").setSelectionRange(lastPosition, lastPosition);
}	
// ------------------------------------------------------------------------