var Visa = require ("../nisa.js").Visa;

var DeviationStream = require('standard-deviation-stream');


// standard Keithley 199 address is 26 
var kei199 = new Visa("GPIB0::26::INSTR", null);

kei199.on('srq', function(a1) {
    console.log(a1 ? ("SRQ 16:  " + a1) : "SRQ Nothing");
});
kei199.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } else {
		kei199.write("M1X", function(err, res) { //enable SRQ
            
            var n = new DeviationStream();
    
            if (err) {
                console.log("open err:  " + err);    
            } else {
                setInterval(function() {
                        kei199.query("G1F0R2X", function (err, res) {
                        var value = Number(res);
                        n.push(value);
                        process.stdout.write(" " + value + "; Mean:" + n.mean() + " sd: " + n.standardDeviation() + "\r");
                    });
                }, 250);
            }    
        }); 
    }
});