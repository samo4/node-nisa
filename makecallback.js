var raw = require('bindings')({ bindings: 'makecallback' });
var EventEmitter = require('events').EventEmitter;

function inherits(target, source) {
    for (var k in source.prototype) {
        target.prototype[k] = source.prototype[k];
    }
}
inherits(raw.VisaEmitter, EventEmitter);

function debugCallback (err, res) {
    if (err) {
        console.log("debugCallback err:  " + err);    
    } else  {
        console.log("debugCallback res:  " + res);
    }
}

var obj16 = new raw.VisaEmitter("GPIB0::16::INSTR");

var obj12 = new raw.VisaEmitter("GPIB0::12::INSTR");

var obj = new raw.VisaEmitter("GPIB0::11::INSTR");

obj16.on('srq', function(a1) {
    console.log(a1 ? ("SRQ 16:  " + a1) : "SRQ Nothing");
});
obj16.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } else {
		obj16.write("M1X", function(err, res) {
            if (err) {
                console.log("open err:  " + err);    
            } else {
                setInterval(function() {
                        obj16.query("F0R2X", function (err, res) {
                        process.stdout.write(" " + res + "\r");
                    });
                }, 1000);
            }    
        }); 
    }
});


/*
obj.on('srq', function(a1) {
    if (a1) {
        console.log("SRQ 11:  " + a1);    
    }
    else {
        console.log("SRQ Nothing");    
    }
});

obj12.on('srq', function(a1) {
    if (a1) {
        console.log("SRQ 12:  " + a1);    
    }
    else {
        console.log("SRQ Nothing");    
    }
});

obj.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } else {
		obj.write("M1X", function(err, res) {
            if (err) {
                    console.log("open err:  " + err);    
                } else {
                        obj.write("D9X", debugCallback);
                }    
        }); 
    }
});

obj12.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } else {
		obj12.write("SRQ 3", function(err, res) {
            if (err) {
                    console.log("open err:  " + err);    
                } else {
                        obj12.write("VSET1,100", debugCallback);
                }    
        }); 
    }
}); 
setTimeout(function() {
    console.log("THE END");
}, 3000);
*/



var keypress = require('keypress');
 
console.log("Press Enter to exit..");
keypress(process.stdin);
 
process.stdin.on('keypress', function (ch, key) {
    console.log("Exiting..");
    process.exit();
    /*if (key && (key.name === 'y' || key.name === 'n')) {
       console.log('Reply:' + key.name);
    }*/
});