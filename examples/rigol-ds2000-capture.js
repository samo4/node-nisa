var async = require('async');
var Visa = require("../nisa.js").Visa;
var rigol = new Visa("TCPIP0::192.168.0.11::inst0::INSTR", { 
    assertREN: false, 
    enableSRQ: false 
  });

async.series([
  function (callback) { rigol.open(callback); },
  //function (callback) { setTimeout(callback, 500) },
  //function (callback) { rigol.deviceClear(callback); },
  function (callback) { rigol.query("*IDN?", function(err, res) {
	if (err) {
		return callback(err);
	}
	console.log(res);
	return callback();
  } ) },
  function (callback) { rigol.write(":DISP:DATA?", callback); },
  function (callback) { rigol.read(11, function(err, res) {
    if (err) {
      return callback(err);
    }
    console.log(res);
    console.log(res.toString('ascii'));
    var numberOfBytesToRead = Number(res.toString('ascii').substring(2));
    rigol.read(numberOfBytesToRead, function(err, buffer) {
      if (err) {
        return callback(err);
      }
      console.log(buffer.length);
      var Jimp = require("jimp");
      var image = new Jimp(buffer, function (err, image) {
        if (!err) {
          image.write("res.jpg"); 
        } 
      });
      
      return callback();
    });
  }) },
  /*
  function (callback) { rigol.read(11, function(err, res) {
	if (err) {
		return callback(err);
	}
  var fs = require('fs');
  fs.writeFile("res.bmp", res, function(err) {
      if(err) {
          return console.log(err);
      }
      console.log("The file was saved!");
  }); 
  
	console.log(res);
	return callback();
  } ) },*/
], function (err, res) {
  if (err) {
    console.log('ERROR');
    console.log(err);
  } else {
    console.log('DONE');
    console.log(res);
  }
});

