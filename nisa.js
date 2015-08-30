var raw = require('bindings')({ bindings: 'nisa' });
var EventEmitter = require('events').EventEmitter;
var util = require ("util");

for (var key in EventEmitter.prototype) {
  raw.VisaEmitter.prototype[key] = EventEmitter.prototype[key];
}

var _options = {
    enableSRQ: true,
    assertREN: true,
	timeoutMiliSeconds: 2000
}

function VisaPort(path, options) {
	VisaPort.super_.call (this);
	
	var self = this;
    options = (typeof options !== 'function') && options || {};
	var opts = {};
	opts.enableSRQ = _options.enableSRQ;
	if (options.hasOwnProperty('enableSRQ')) {
		opts.enableSRQ = options.enableSRQ;
	}
	opts.assertREN = _options.assertREN;
	if (options.hasOwnProperty('assertREN')) {
		opts.assertREN = options.assertREN;
	}
	opts.timeoutMiliSeconds = _options.timeoutMiliSeconds;
	if (options.hasOwnProperty('timeoutMiliSeconds') && options.timeoutMiliSeconds > 100) {
		opts.timeoutMiliSeconds = options.timeoutMiliSeconds;
	}
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
			me.emit ("error", err);
		}  
		return callback(err, res);
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

VisaPort.prototype.read = function (numberOfBytes, callback) {
	var args = Array.prototype.slice.call(arguments);
    callback = args.pop();
    if (typeof (callback) !== 'function') {
      callback = null;
    }
	numberOfBytes = args.pop();
	if (typeof (numberOfBytes) !== 'number') {
      numberOfBytes = 256;
    }
	this.wrap.read(numberOfBytes, callback);
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

VisaPort.prototype.close = function(callback) {
	this.wrap.close(callback);
	return this;
}

VisaPort.prototype.onSrq = function (stb) {
	var me = this;
	me.emit ("srq", stb);
}

exports.Visa = VisaPort;
exports.version = '1.0.0';