"use strict";
// Client-side interactions with the browser.

// Web sockets: automatically establish a socket with the server
var socket = io.connect();

// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	// Make the text-entry box have keyboard focus.
	socket.on('daResponse', function(result) {
		console.log(result);
	});

	// Handle data coming back from the server
	socket.on('daAnswer', function(result) {
		$('#messages').append(divMessage(result));
	});
	
	socket.on('daError', function(result) {
		var msg = divMessage('SERVER ERROR: ' + result);
		$('#messages').append(msg);
	});
	
});

function setModeNone() {
    console.log("Setting mode to none -local");
}

function setModeRock1() {
    console.log("Setting mode to Rock1 -local");
}

function setModeRock2() {
    console.log("Setting mode to Custom Rock -local");
}

function volUp() {
	console.log("Raising volume -local");
	socket.emit('volUp');
}

function volDown() {
	console.log("Lowering volume -local");
}