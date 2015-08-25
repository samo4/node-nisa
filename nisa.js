var raw = require('bindings')({ bindings: 'nisa' });
var EventEmitter = require('events').EventEmitter;
var util = require ("util");

for (var key in EventEmitter.prototype) {
  raw.VisaEmitter.prototype[key] = EventEmitter.prototype[key];
}

var _options = {
    enableSRQ: true,
    assertREN: true
}

function VisaPort(path, options) {
	VisaPort.super_.call (this);
	
	var self = this;
    options = (typeof options !== 'function') && options || {};
	var opts = {};
	opts.enableSRQ = options.enableSRQ || _options.enableSRQ;
	opts.assertREN = options.assertREN || _options.assertREN;
	this.options = opts;
	this.wrap = new raw.VisaEmitter(path);
	this.wrap.on ("srq", this.onSrq.bind (self));
}

util.inherits (VisaPort, EventEmitter);

VisaPort.prototype.open = function (callback) {
	var me = this;
	
	this.wrap.open(this.options, function(err, res) {
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

VisaPort.prototype.trigger = function(callback) {
	this.wrap.trigger(callback);
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