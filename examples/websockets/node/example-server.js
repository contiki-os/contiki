"use strict";

var serverPort = 8080;

var websocket = require('websocket').server;
var http = require('http');

var server = http.createServer(function(request, response) {
    response.writeHead(200, {'Content-Type': 'text/plain'});
    response.write('This is a websocket server, not intended for http\n');
    response.end();
});

server.listen(serverPort, function() {
    console.log('Server is listening on port ' + serverPort);
});

var wsServer = new websocket({
    httpServer: server
});


var connections = [];

function broadcastMessage(message) {
    for (var i = 0; i < connections.length; i++) {
        connections[i].sendUTF(message);
    }
}

wsServer.on('request', function(request) {
    /* Save the connection */
    var connection = request.accept(null, request.origin);

    /* Store the connection in the list of connections */
    var connectionIndex = connections.push(connection) - 1;

    console.log('Connection from ' + connection.remoteAddress + '.');

    broadcastMessage('Connection from ' + connection.remoteAddress + '.');

    connection.on('message', function(message) {
        if (message.type === 'utf8') {
            console.log((new Date()) + ' Message received: ' +
                        message.utf8Data);
            broadcastMessage(message.utf8Data);
        }
    });

    // user disconnected
    connection.on('close', function(connection) {
        console.log((new Date()) + ' Connection lost: ' +
                    connection.remoteAddress);
        connections.splice(connectionIndex, 1);
    });

});
