var async = require('async');
var Visa = require ("../nisa.js").Visa;

var DeviationStream = require('standard-deviation-stream');

// standard Keithley 199 address is 26 
var kei199 = new Visa("GPIB0::26::INSTR");

kei199.on('srq', function(stb) {
    if (!stb)
        return;
    if (stb & 1) 
        console.log("Reading overflow");
    if (stb & 2) 
        console.log("Data store full");
    if (stb & 4) 
        console.log("Data store half full");
    if (stb & 8) 
        console.log("Reading done");
        
    if (!(stb & 15)) {
        console.log("Unknown SRQ status byte (we really should add &40 to C:  " + (stb).toString(2));    
    } 
});

async.series ([
  function(callback) { kei199.open(callback); }, 
  function(callback) { kei199.query("O0G1F0R2X", callback) }, // 2-pole; scientific notation; DCV 3V;  
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { kei199.query("N18Q1000T2X", callback) }, // one channel per store interval; 1s interval; continious GET
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { kei199.query("I80M2X", callback) }, // 80 readigns; SRQ when full
  function(callback) { setTimeout(callback, 500) }  
], function(err, res) {
   if (err) { 
     console.log('ERROR');
     console.log(err);
   } else {
     console.log('DONE');
     console.log(res);
   }   
});

