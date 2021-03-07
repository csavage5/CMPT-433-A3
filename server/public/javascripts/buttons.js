"use strict";
// Client-side interactions with the browser.

// Web sockets: automatically establish a socket with the server
var socket = io.connect();
var errorTimer

// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	// Make the text-entry box have keyboard focus.
	socket.on('daResponse', function(result) {
		console.log(result);
		clearTimeout(errorTimer);
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
    console.log("Sending none request");
	// error timer starts here
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('setNone');
}

function setModeRock1() {
    console.log("Sending rock1 request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('setRock1');
}

function setModeRock2() {
    console.log("Sending rock2 request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('setRock2');
}

function volDown() {
	console.log("Sending lower volume request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('volDown');
}

function volUp() {
	console.log("Sending raise volume request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('volUp');
}

function tempDown() {
	console.log("Sending lower tempo request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('tempoDown');
}

function tempUp() {
	console.log("Sending raise tempo request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('tempoUp');
}

function playHiHat() {
	console.log("Sending HiHat request");
	socket.emit('playHiHat');
}

function playSnare() {
	console.log("Sending snare request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('playSnare');
}

function playBase() {
	console.log("Sending Base request");
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
	}, 5000);
	socket.emit('playBase');
}