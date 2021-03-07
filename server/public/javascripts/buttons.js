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
		document.getElementById("error-box").style.display = "none";
	});


	
});

function setErrorTimer() {
	errorTimer = setTimeout(function() {
		socket.emit("serverTimeout", "Connection timeout to server");
		console.log("Server did not respond in time. Ensure server.js is running")
		document.getElementById("error-box").style.display = "block";
	}, 5000);
}

function setModeNone() {
    console.log("Sending none request");
	// error timer starts here
	setErrorTimer();
	socket.emit('setNone');
}

function setModeRock1() {
    console.log("Sending rock1 request");
	setErrorTimer();
	socket.emit('setRock1');
}

function setModeRock2() {
    console.log("Sending rock2 request");
	setErrorTimer();
	socket.emit('setRock2');
}

function volDown() {
	console.log("Sending lower volume request");
	setErrorTimer();
	socket.emit('volDown');
}

function volUp() {
	console.log("Sending raise volume request");
	setErrorTimer();
	socket.emit('volUp');
}

function tempDown() {
	console.log("Sending lower tempo request");
	setErrorTimer();
	socket.emit('tempoDown');
}

function tempUp() {
	console.log("Sending raise tempo request");
	setErrorTimer();
	socket.emit('tempoUp');
}

function playHiHat() {
	console.log("Sending HiHat request");
	setErrorTimer();
	socket.emit('playHiHat');
}

function playSnare() {
	console.log("Sending snare request");
	setErrorTimer();
	socket.emit('playSnare');
}

function playBase() {
	console.log("Sending Base request");
	setErrorTimer();
	socket.emit('playBase');
}