var async = require('async');
var Visa = require("../nisa.js").Visa;

var DeviationStream = require('standard-deviation-stream');

// standard Keithley 199 address is 26 
var kei199 = new Visa("GPIB0::26::INSTR");

var numberOfMeasurements = 20;

kei199.on('srq', function (stb) {
  if (!stb)
    return;
  if (stb & 1)
    console.log("Reading overflow");
  if (stb & 2) {
    async.series([
      function (callback) { kei199.write("B1N10X", callback) }, // data from store, stop scan  
      function (callback) { kei199.write("G6X", callback) }, // format with locations, channels
      function (callback) {
        var items = new Array(numberOfMeasurements); // [ 1, 2, 3, 4 ];
        async.forEachOf(items, function (v, k, next) {
          kei199.read(function (err, res) {
            var scientificNotation = /([+/-]\d+\.\d+[Ee][+/-]\d+)(,[B](\d{0,3})){0,1}(,[C](\d{0,3})){0,1}/;
            var match = scientificNotation.exec(res.toString('ascii'));
            items[k] = new Array(4);
            items[k][0] = Number(match[3]);
            items[k][1] = Number(match[5]);
            items[k][2] = Number(match[1]);
            items[k][3] = res.toString('ascii'); 
            return next();
          });
        }, function (err) {
          if (err) {
            console.log("error reading: " + err);
            return callback(err);
          }
          console.log(items);
          return callback();
        });
      },
      function (callback) { kei199.close(callback) }, 
    ], function (err, res) {
      if (err) {
        console.log('ERROR1');
        console.log(err);
      } else {
        console.log('DONE1');
        console.log(res);
      }
    });
  }
  if (stb & 4)
    console.log("Data store half full");
  if (stb & 8)
    console.log("Reading done");

  if (!(stb & 15)) {
    console.log("Unknown SRQ status byte (we really should add &40 to C:  " + (stb).toString(2));
  }
});

async.series([
  function (callback) { kei199.open(callback); },
  //function (callback) { setTimeout(callback, 500) },
  function (callback) { kei199.deviceClear(callback); },
  function (callback) { kei199.write("M1X", callback) }, // 2-pole; scientific notation; DCV 3V;  
  function (callback) { kei199.write("O0G1F0R0X", callback) }, // 2-pole; scientific notation; DCV 3V;  
  function (callback) { kei199.write("N0Q250T2X", callback) }, // one channel per store interval; 250ms interval; continious GET
  function (callback) { kei199.write("I" + numberOfMeasurements + "X", callback) },// 4 readigns;
  function (callback) { kei199.write("M3X", callback) },  // SRQ when full and on overrange
  function (callback) { setTimeout(callback, 500) },
  function (callback) { kei199.trigger(callback) } // ATN low; address; GET command byte // probably everything is done by viAssertTrigger
  /*function(callback) {
        setInterval(function() {
                        kei199.query("G1F0R2X", function (err, res) {
                        var value = Number(res);
                        process.stdout.write(" " + value + ";\r");
                    });
                }, 250);
  }*/
], function (err, res) {
  if (err) {
    console.log('ERROR');
    console.log(err);
  } else {
    console.log('DONE');
    console.log(res);
  }
});

/*
async.series ([
  function(callback) { kei199.open(callback); }, 
  function(callback) { kei199.write("O0G1F0R2X", callback) }, // 2-pole; scientific notation; DCV 3V;  
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { kei199.write("N18Q1000T2X", callback) }, // one channel per store interval; 1s interval; continious GET
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { kei199.write("I8M3X", callback) }, // 80 readigns; SRQ when full and on overrange
  function(callback) { setTimeout(callback, 500) },
  function(callback) { kei199.trigger(callback) } // ATN low; address; GET command byte // probably everything is done by viAssertTrigger
], function(err, res) {
   if (err) { 
     console.log('ERROR');
     console.log(err);
   } else {
     console.log('DONE');
     console.log(res);
   }   
}); */

