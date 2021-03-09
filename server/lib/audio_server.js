/*
 * Respond to commands over a websocket to do math
 */

const { SSL_OP_EPHEMERAL_RSA } = require('constants');
var dgram = require('dgram');
var client = dgram.createSocket('udp4');
var timeClient = dgram.createSocket('udp4');

var socketio = require('socket.io');
var io;
var newTime = '00:00:00';
var currentTime = '00:00:00';
var timeout = false;

// handle incoming UDP
client.on('listening', () => {
    const address = client.address();
    console.log(`server listening on ${address.address}:${address.port}`);
})

timeClient.on('listening', () => {
    const address = timeClient.address();
    console.log(`server listening on ${address.address}:${address.port}`);
    });

timeClient.on('message', (msg, senderInfo) => {
    // Translate from byte array to string
    msg = String.fromCharCode.apply(null,msg);
    newTime = msg;
    clearTimeout(bbgErrorTimer);
    timeout = false;
});

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
	
	
};

function handleCommand(socket) {
	socket.on('setNone', function(){
        var message = 'Setting mode to none';
		client.send('p beat 0',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('setRock1', function(){
        var message = 'Setting mode to Rock1';
		client.send('p beat 1',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('setRock2', function(){
        var message = 'Setting mode to Rock2';
		client.send('p beat 2',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('volDown', function(){
        var message = 'Lowering volume';
		client.send('vol d 5',12345, 'localhost');
        client.on('message', (msg, senderInfo) => {
            // Translate from byte array to string
            msg = String.fromCharCode.apply(null,msg);
            socket.emit('volumeControl', msg);
        });
        socket.emit('daResponse', message);
    });

	socket.on('volUp', function(){
        var message = 'Raising volume';
		client.send('vol u 5',12345, 'localhost');
        client.on('message', (msg, senderInfo) => {
            // Translate from byte array to string
            msg = String.fromCharCode.apply(null,msg);
            socket.emit('volumeControl', msg);
        });
        socket.emit('daResponse', message);
    });

    socket.on('tempoDown', function(){
        var message = 'Lowering tempo';
		client.send('tempo d 5',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('tempoUp', function(){
        var message = 'Raising tempo';
		client.send('tempo u 5',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('playHiHat', function(){
        var message = 'Raising tempo';
		client.send('p drum hh',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('playSnare', function(){
        var message = 'Raising tempo';
		client.send('p drum snare',12345, 'localhost');
        socket.emit('daResponse', message);
    });

	socket.on('playBase', function(){
        var message = 'Playing Base';
		client.send('p drum base',12345, 'localhost');
        socket.emit('daResponse', message);
    });

    socket.on('timeRequest', function(){
        timeClient.send('q uptime',12345, 'localhost');
		updateTime(socket)
        if(timeout == false){
            bbgErrorTimer = setTimeout(function() {
                    socket.emit('backendTimeout');
                    timeout = true;
            }, 3000);
        }
    });
}

function updateTime(socket) {
    // hopefully temp fix, send request for new time, 
    // wait 200ms for update then send current time
    setTimeout(function() {
        if(currentTime != newTime){
            currentTime = newTime;
            socket.emit('uptime', currentTime);
        }
    }, 100);
}
