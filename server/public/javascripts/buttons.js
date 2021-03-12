"use strict";
// Client-side interactions with the browser.

// Web sockets: automatically establish a socket with the server
var socket = io.connect();
var errorTimer

// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	socket.on('daResponse', function(result) {
		console.log(result);
		clearTimeout(errorTimer);
		document.getElementById('error-box').style.display = 'none';
		document.getElementById('error1').style.display = 'none';
	});

	socket.on('uptime', function(result) {
		$('#timer').html(result + '(H:M:S)');
		clearTimeout(errorTimer);
		document.getElementById('error-box').style.display = 'none';
		document.getElementById('error2').style.display = 'none';
	});

	socket.on('volumeControl', function(result) {
		console.log(result)
		document.getElementById('volumeid').value = result;
	});

	socket.on('tempoControl', function(result) {
		console.log(result)
		document.getElementById('tempoid').value = result;
	});

	socket.on('backendTimeout', function() {
		console.log('Backend is not responding, check that it is running')
		document.getElementById('error-box').style.display = 'block';
		document.getElementById('error2').style.display = 'block';
	})

	window.setInterval(function() {updateTime()}, 1000);
});

function setErrorTimer() {
	errorTimer = setTimeout(function() {
		socket.emit('serverTimeout', 'Connection timeout to server');
		console.log('Server did not respond in time. Ensure server.js is running')
		document.getElementById('error-box').style.display = 'block';
		document.getElementById('error1').style.display = 'block';
	}, 3000);
}

function updateTime() {
	socket.emit('timeRequest');
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

function getVol() {
	console.log("Sending volume update request");
	setErrorTimer();
	socket.emit('getVol');
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

function getTempo() {
	console.log("Sending tempo update request");
	setErrorTimer();
	socket.emit('getTempo');
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

function playBass() {
	console.log("Sending Bass request");
	setErrorTimer();
	socket.emit('playBass');
}