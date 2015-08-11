const bindings = require('bindings')({  bindings: 'nancallback' });
    const round = Math.round;


  var persistent = bindings;
  persistent.globalContext(function () { console.log(true); });
  persistent.specificContext(function () { console.log(true); });
  // persistent.customReceiver(function () { t.equal(this, process); }, process);
  persistent.callDirect(function () { console.log(true); });
  persistent.callAsFunction(function () { console.log(true); });

  var round2 = Math.round
    , x = function(param) { return param + 1; }
    , y = function(param) { return param + 2; }
    , x2 = x;
  persistent.compareCallbacks(round, round2, Math.floor);
  persistent.compareCallbacks(x, x2, y);
