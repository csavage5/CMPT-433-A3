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
    socket.on('volUp', function(){
        var message = 'Raise volume received';
        socket.emit('daResponse', message);
    });
}