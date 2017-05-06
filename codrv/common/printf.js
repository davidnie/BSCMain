// printf over CAN
//
// Copyright (c) 2013 emtas GmbH
//
// $Id: $
//

// helper function
function dateTime()
{
var currentDateTime = new Date();
var hours = currentDateTime.getHours();
var minutes = currentDateTime.getMinutes();
var seconds = currentDateTime.getSeconds();
var milli_seconds = currentDateTime.getMilliseconds();
var day = currentDateTime.getDate();
var month = currentDateTime.getMonth() + 1;
var year = currentDateTime.getFullYear();

var out = "";

	//date
	if (day < 10) {day = "0" + day;}      
	if (month < 10) {month = "0" + month;}
	out = out + "<b>" +"  @ "  + day + "." + month + "." + year + "</b>";

      // add time 
	if (milli_seconds < 10) {milli_seconds = "00" + milli_seconds;} 
      else if (milli_seconds < 100) {milli_seconds = "0" + milli_seconds;} 
	if (seconds < 10) {seconds = "0" + seconds;}
	if (minutes < 10) {minutes = "0" + minutes;}
	if (hours < 10) {hours = "0" + hours;}  

	out = out + " " + "<b>" + hours + ":" + minutes+ ":" + seconds + ","+ milli_seconds+  " " + "</b>";

	return out;       
}

//-------------------------------------------------------------------

// used CAN ID
var printf_id = 0x580
var printf_id = 0x001
var printf_id = 0x002

// reset all old setting
can.unregisterAllCanEvents();
util.deleteAllTimers();

// global String (if more than one CAN Message per String)
var output = new String("");
var timeout_timer = 0;

// callback function
function view_data(id, rtrFlag, dlc, d0, d1, d2, d3, d4, d5, d6, d7)
{
var data = new Array( d0, d1, d2, d3, d4, d5, d6, d7);
var nl = new Array(0, 10, 13); //newline chars
var i;
var c;

	if (timeout_timer != 0) {
		util.deleteTimer(timeout_timer);
		timeout_timer = 0;
	}

	for (i = 0; i < dlc; i++) {
		if ( nl.indexOf(data[i]) > -1) {
			util.print(dateTime()+output);
			output = "";
		} else {
			c = String.fromCharCode(data[i]);
			//util.print(c);
			output = output + c;

			//set timeout 1s
			if (timeout_timer == 0) {
				timeout_timer = util.every(1000, "timeout()");
			}
		}
	}
}

// timeout in case of no newline char
function timeout()
{
	util.print(dateTime() + output);
	output = "";

	// simulate one shoot timer
	util.deleteTimer(timeout_timer);
	timeout_timer = 0;

}

// initialization
function init_printf_view()
{
	can.registerCanEvent( printf_id, "view_data" );
}
init_printf_view();

util.print("script loaded");

//end
