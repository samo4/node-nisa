# node-nisa

nisa is a working prototype for using VISA library in node.js.
This is a wrapper for visa32.dll (in your system32 folder).

I tested this code on Windows 8.1 (64bit), Keysight IO Library (17.1), node.js

Talked to HP 6623A, Keithley 199, Keithley 228A, Keithley 237

You must have Keysight or National Instruments VISA drivers installed. You need to have compatible interface and and instrument that talks over GPIB. For simple testing you can skip the hardware part as VISA can talk over COM ports, too. 
```
> npm install node-nisa

var visa = require('nisa');

visa.query("GPIB0::12::INSTR", "*IDN?", function(err, result) {
 // do something
}
```
## Known limitations and bugs
Due to lack of interest (it works for me) and more importantly lack of knowledgem, the current implementation includes some memory leaks.
The implemented functions were selected solely to control my current setup. This is not meant to be a complete implementation of VISA interface. API was adapted to my undestanding of node ways to my best efforts.

## Acknowledgements

A lot of the code was inspired by (or blatantly copied) from:
- https://github.com/voodootikigod/node-serialport
and
- https://bitbucket.org/stephenwvickers/node-raw-socket
Renewed interest was inspired by 7M4MON and forking his ffi implementation (still a viable approach):
- https://github.com/7m4mon/visa32test
