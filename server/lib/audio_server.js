/*
 * Respond to commands over a websocket to do math
 */

var socketio = require('socket.io');
var io;

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
        socket.emit('daResponse', message);
    });

	socket.on('setRock1', function(){
        var message = 'Setting mode to Rock1';
        socket.emit('daResponse', message);
    });

	socket.on('setRock2', function(){
        var message = 'Setting mode to Rock2';
        socket.emit('daResponse', message);
    });

	socket.on('volDown', function(){
        var message = 'Lowering volume';
        socket.emit('daResponse', message);
    });

	socket.on('volUp', function(){
        var message = 'Raising volume';
        socket.emit('daResponse', message);
    });

    socket.on('tempoDown', function(){
        var message = 'Lowering tempo';
        socket.emit('daResponse', message);
    });

	socket.on('tempoUp', function(){
        var message = 'Raising tempo';
        socket.emit('daResponse', message);
    });
}