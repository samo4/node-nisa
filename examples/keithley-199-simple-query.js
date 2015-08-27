var Visa = require ("../nisa.js").Visa;

var DeviationStream = require('standard-deviation-stream');

// standard Keithley 199 address is 26 
var kei199 = new Visa("GPIB0::26::INSTR", null);

kei199.on('srq', function(stb) {
    if (!stb)
        return;
    // console.log(stb ? ("SRQ 16:  " + stb.toString(2)) : "SRQ Nothing");
    
    if (stb & 1) 
        console.log("Reading overflow");
    if (stb & 2) 
        console.log("Data store full");
    if (stb & 4) 
        console.log("Data store half full");
    if (stb & 8) 
        console.log("Reading done");
        
    if (!(stb & 15)) {
        console.log("Unknown SRQ status byte:  " + (stb).toString(2));    
    } 
});

kei199.on('open', function(res) {
   console.log("recieved open event.." + res);
   kei199.deviceClear(function (err, res) {
        if (!err) {
            kei199.write("M1X", function(err, res) { //enable SRQ    
                var n = new DeviationStream();
        
                if (err) {
                    console.log("open err:  " + err);    
                } else {
                    setInterval(function() {
                            kei199.query("G1F0R0X", function (err, res) {
                            var value = Number(res);
                            n.push(value);
                            process.stdout.write(" " + value + "; Mean:" + n.mean() + " sd: " + n.standardDeviation() + "\r");
                        });
                    }, 250);
                }    
            });
        }
   });
});
kei199.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } 
});