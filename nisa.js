var raw = require('bindings')({ bindings: 'makecallback' });
var EventEmitter = require('events').EventEmitter;
var util = require ("util");

for (var key in EventEmitter.prototype) {
  raw.VisaEmitter.prototype[key] = EventEmitter.prototype[key];
}

function VisaPort(path, options) {
	VisaPort.super_.call (this);
	this.wrap = new raw.VisaEmitter(path);
	var me = this;
	this.wrap.on ("srq", this.onSrq.bind (me));
}

util.inherits (VisaPort, EventEmitter);

VisaPort.prototype.open = function (callback) {
	var me = this;
	
	this.wrap.open(function(err, res) {
		if (!err) {
			me.emit ("open", res);
		} else {
			console.log("ERR: ", err);
		}  
		return callback();
	});
	return this;
}

VisaPort.prototype.write = function (query, callback) {
	this.wrap.write(query, callback);
	return this;
}

VisaPort.prototype.query = function (query, callback) {
	this.wrap.query(query, callback);
	return this;
}

VisaPort.prototype.read = function (callback) {
	this.wrap.read(callback);
	return this;
}

VisaPort.prototype.trigger = function() {
	this.wrap.trigger();
	return this;
}

VisaPort.prototype.deviceClear = function(callback) {
	this.wrap.deviceClear(callback);
	return this;
}

VisaPort.prototype.onSrq = function (stb) {
	var me = this;
	me.emit ("srq", stb);
}

exports.Visa = VisaPort;
exports.version = '1.0.0';