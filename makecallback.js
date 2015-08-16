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
    } else {
        console.log("debugCallback res: " + res);    
    }
}

var obj = new raw.VisaEmitter("GPIB0::11::INSTR");

obj.on('ping', function(a1) {
    if (a1) {
        console.log("ping:  " + a1);    
    }
    else {
        console.log("ping Nothing");    
    }
});

obj.on('srq', function(a1) {
    if (a1) {
        console.log("SRQ:  " + a1);    
    }
    else {
        console.log("SRQ Nothing");    
    }
});

obj.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } else {
        console.log("open....");
		obj.write("M1X", function(err, res) {
            if (err) {
                    console.log("open err:  " + err);    
                } else {
                        obj.write("D9X", debugCallback);
                }    
        }); 
    }
});

setTimeout(function() {
    console.log("THE END");
}, 3000);

