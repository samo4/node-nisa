var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var Visa = require ("../nisa.js").Visa;
var path = require('path');
var kei199 = new Visa("GPIB0::26::INSTR", null);
var async = require('async');

app.get('/',function(req,res){
    res.sendFile(path.join(__dirname + '/keithley-199-express.html'));
});
var rcvMsg;

function readResult(err, res) {
	if (err)
		io.emit('recvmsg', err);
	else
	{
		var result = res.toString('ascii');
		var scientificNotation = /([+/-]\d+\.\d+[Ee][+/-]\d+)(,[B](\d{0,3})){0,1}(,[C](\d{0,3})){0,1}/;
		var match = scientificNotation.exec(result);
		console.log(result);
		console.log( match[1]);
		io.emit('recvmsg', count++ + " " + Number(match[1]));
	}
}

var count = 0;
kei199.on('open', function(res) {
	console.log("k199 opened.." + res);
	kei199.deviceClear(function (err, res) {
		
		io.sockets.on('connection', function (socket) {
			socket.on('send', function (data) {
				io.sockets.emit('message', data);
			});
			socket.on('sendmsg',function(msg){
				console.log("sendmsg.. " + msg.cmd);
				kei199.write(msg.cmd, function(err, res) { if (!err) { console.log("OK");} });
			});
			async.forever(
				function (next) {
					kei199.read(18, function readResult(err, res) {
						if (!err) {
							var result = res.toString('ascii');
							var scientificNotation = /([+/-]\d+\.\d+[Ee][+/-]\d+)(,[B](\d{0,3})){0,1}(,[C](\d{0,3})){0,1}/;
							var match = scientificNotation.exec(result);
							if (match) {
								console.log(match[1]);
								socket.emit('message', { message: Number(match[1]) });	
							} else {
								socket.emit('message', { message: result });
								console.log(result);
							}
							next();
						}	
						else {
							console.log(err);
						}
					});
				}
			);
		});
		/*
		io.on('connection',function(socket) {
			//socket.emit('message', { message: 'welcome to the chat' });
			
		}); */
	});
});

kei199.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } 
});

http.listen(3000,function(){
    console.log('listen 3000 port');
});