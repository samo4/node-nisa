var memwatch = require('memwatch-next');
var async = require('async');
var Visa = require("../nisa.js").Visa;
var rigol = new Visa("TCPIP0::192.168.0.11::inst0::INSTR", { 
    assertREN: false, 
    enableSRQ: false,
    timeoutMiliSeconds: 1000
  });
  
function displayResult (err, res) {
  if (err) {
    console.log('ASYNC ERROR:');
    console.log(err);
  } else {
    console.log('ASYNC DONE:');
    console.log(res);
  }
}

memwatch.on('leak', function(info) { console.log(info); });
memwatch.on('stats', function(stats) { console.log(stats); });

async.series([
  function (callback) { rigol.open(callback); },
  //function (callback) { rigol.query("*IDN?", callback); },
  //function (callback) { rigol.write(":DISP:DATA?", callback); },
  function (callback) { 
    async.forever(
        function (next) { 
          async.series([
            function (callback) { rigol.write(":DISP:DATA?", callback); },
            function (callback) { rigol.read(11, function(err, res) {
              if (err) {
                return callback(err);
              }
              var numberOfBytesToRead = Number(res.toString('ascii').substring(2));
              rigol.read(numberOfBytesToRead, function(err, buffer) {
                if (err) {
                  return callback(err);
                }
                console.log("Got: " + buffer.length + " bytes");
                var Jimp = require("jimp");
                new Jimp(buffer, function (err, image) {
                  if (!err) {
                    image.write("res.jpg", function(err) {if (!err) { console.log("got another one"); }}); 
                  } 
                });
                return callback();
              });
            }) },
          ], function (err, res) {
              if (!err) { 
               return next();
              }
          });
        }
    );
  }
], displayResult);


/*


        
*/ 