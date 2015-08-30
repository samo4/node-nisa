var Visa = require ("../nisa.js").Visa;


var influx = require('influx')

var client = influx({
  host : 'medon.netinet.si',
  port : 8086, // optional, default 8086
  protocol : 'http', // optional, default 'http'
/*  username : 'test',
  password : 'mL}E-dTa"kc7rRbA',
  */
  database : 'k199'
})



client.query('k199', 'SELECT mean(value) FROM test WHERE time > now() - 10m GROUP BY time(1m)', function (err, res) {
    if (err) { 
        console.log(err); 
    } 
    else {
        console.log(res);
    }
} )

// standard Keithley 199 address is 26 
var kei199 = new Visa("GPIB0::26::INSTR", null);

kei199.on('srq', function(stb) {
    if (!stb)
        return;
    if (stb & 1) 
        console.log("Reading overflow");
    if (stb & 2) 
        console.log("Data store full");
    if (stb & 4) 
        console.log("Data store half full");
    if (stb & 8) 
        console.log("Reading done");
    if (!(stb & 15)) {
        console.log("Unknown SRQ status byte:  " + (stb).toString(2));    
    } 
});

kei199.on('open', function(res) {
   console.log("recieved open event.." + res);
   kei199.deviceClear(function (err, res) {
        if (!err) {
            kei199.write("M1X", function(err, res) { //enable SRQ    
        
                if (err) {
                    console.log("open err:  " + err);    
                } else {
                    setInterval(function() {
                            kei199.query("G1F0R0X", function (err, res) {
                            var value = Number(res);
                            process.stdout.write(" " + value + ";writing..\r");
                            client.writePoint("test", {time: new Date(), value: value}, null, {database: 'k199'}, function (err, res) {
                                if (err) {
                                    console.log("err.." + err);
                                }
                                
                            });
                            
                        });
                    }, 10000);
                }    
            });
        }
   });
});
kei199.open(function (err, res) {
    if (err) {
        console.log("open err:  " + err);    
    } 
});