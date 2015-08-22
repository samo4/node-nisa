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

VisaPort.prototype.open = function (err, res) {
	this.wrap.open(err, res);
	return this;
}

VisaPort.prototype.write = function (err, res) {
	this.wrap.write(err, res);
	return this;
}

VisaPort.prototype.query = function (err, res) {
	this.wrap.query(err, res);
	return this;
}

VisaPort.prototype.onSrq = function (stb) {
	var me = this;
	me.emit ("srq", stb);
}

exports.Visa = VisaPort;
exports.version = '1.0.0';