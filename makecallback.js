var raw = require('bindings')({ bindings: 'makecallback' });
var EventEmitter = require('events').EventEmitter;

function inherits(target, source) {
    for (var k in source.prototype) {
        target.prototype[k] = source.prototype[k];
    }
}
inherits(raw.VisaEmitter, EventEmitter);

var obj = new raw.VisaEmitter();

obj.on('srq', function(a1) {
    if (a1)
    {
        console.log("SRQ:  " + a1);    
    }
    else
    {
        console.log("Nothing");    
    }
});

obj.ping("pong");

setTimeout(function() {
    console.log("THE END");
}, 3000);

