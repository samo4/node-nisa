
var MyObject = require('bindings')({ bindings: 'makecallback' }).MyObject
var EventEmitter = require('events').EventEmitter;

// extend prototype
function inherits(target, source) {
    for (var k in source.prototype) {
        target.prototype[k] = source.prototype[k];
    }
}
inherits(MyObject, EventEmitter);

var obj = new MyObject();



obj.on('event', function() {
    console.log("OK")
});

obj.call_emit();
