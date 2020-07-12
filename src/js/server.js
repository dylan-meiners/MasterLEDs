var express = require('express')

var app = express()
var server = app.listen(3000)

app.use(express.static('public'))

console.log("Server running...")

var socket = require('socket.io')
const { execFile } = require('child_process')
var io = socket(server)

io.sockets.on('connection', newConnection)
var sockets = {}
var directory = {}

function newConnection(socket) {
    sockets[socket.id] = {
        "socket": socket,
        "validated": false,
    }
    console.log("New connection: " + socket.id)
    
    socket.on("disconnect", function() {
        console.log(socket.id + " disconnected")
        if ("j" in directory) {
            sendMessage(socket.id, "j", "message-zbcs") //z for connection change, g for connect, cs for c++ to server
        }
    })
    socket.on("message", function(msg) {
        messageEvent(socket, msg)
    })
}

function messageEvent(sender, msg) {
    switch (msg.slice(0, 1)) {
        case "c":
            sendMessage(sender.id, "c", msg.slice(1, msg.length))
            break;
        case "j":
            sendMessage(sender.id, "j", msg.slice(1, msg.length))
            break
        case "s":
            sendMessage("s")
            break
        case "v":
            switch(msg.slice(1, msg.length)) {
                case "c":
                    sockets[sender.id]["validated"] = true
                    directory["c"] = sender.id
                    console.log("Validated c++")
                    sendMessage("s", "j", "message-zgcs")
                    sendMessage("s", "c", "message-hello")
                    break
                case "j":
                    sockets[sender.id]["validated"] = true
                    directory["j"] = sender.id
                    console.log("Validated javascript")
                    sendMessage("s", "j", "message-zgjs")
                    const { backend } = require('child_process').execFile;
                    const bchild = execFile('Backend\\Backend.exe');
                    break
                default:
                    break
            }
            break
        default:
            console.log("Invalid message from " + sender.id + ": " + msg)
            break
    }
}

function sendMessage(sender_id, to, msg_cut) {
    //console.log(msg_cut)
    var ok = false;
    if (sender_id == "s") {
        ok = true;
    }
    else if (sockets[sender_id]["validated"] && directory[to] != undefined) ok = true
    if (ok) {
        sockets[directory[to]]["socket"].emit(
            msg_cut.slice(0, msg_cut.indexOf('-')),
            msg_cut.slice(msg_cut.indexOf("-") + 1, msg_cut.length))
    }
    else {console.log(sender_id + " tried to send a message but is not validated, or the destination is not validated")}
}