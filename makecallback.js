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
    } 
}

var obj12 = new raw.VisaEmitter("GPIB0::12::INSTR");

var obj = new raw.VisaEmitter("GPIB0::11::INSTR");

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

