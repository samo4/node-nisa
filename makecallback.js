
/*
var MyObject = require('bindings')({ bindings: 'makecallback' }).MyObject


// extend prototype

var obj = new MyObject();

obj.on('event', function() {
    console.log("OK")
});

obj.call_emit();*/

function mujo()
{
    console.log("mujo");
}

var VisaEmitter = require('bindings')({ bindings: 'makecallback' }).VisaEmitter;
var EventEmitter = require('events').EventEmitter;

// extend prototype
function inherits(target, source) {
    for (var k in source.prototype) {
        target.prototype[k] = source.prototype[k];
    }
}
inherits(VisaEmitter, EventEmitter);

var obj = new VisaEmitter();

obj.on('event', function(e) {
    console.log("Event fired!");
    console.log(e);
    
});

obj.ping("pong");

setTimeout(function() {
    console.log("THE END");
}, 3000);

