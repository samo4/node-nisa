#ifndef _H_MAKE_CALLBACK_
#define _H_MAKE_CALLBACK_

// source: https://bitbucket.org/stephenwvickers/node-raw-socket

// another potential: https://gitlab.com/no9/udt4/tree/master/src
// and another: https://github.com/bramp/nodewii/blob/master/src/wiimote.cc
// more aysnc: http://blog.trevnorris.com/2013/07/node-with-threads.html http://bramp.github.io/nodewii-talk
// and: http://blog.trevnorris.com/2013/07/node-with-threads.html

// and this one when we're ready for 2.x https://github.com/nodejs/nan/issues/376#issuecomment-120838432



#include <nan.h>
#include <visa.h>

using namespace v8;

namespace raw {

// rename to VisaEmitterWrap?
  class VisaEmitter : public node::ObjectWrap {
    public:
      void HandleIOEvent (int status, int revents);
      static void Init();
    private:
      VisaEmitter();
      ~VisaEmitter();
      
      int Connect (void);
    
      static NAN_METHOD(New);
      static NAN_METHOD(Ping);
      
      uv_poll_t *poll_watcher_;
      bool poll_initialised_;
  }; 

  static void IoEvent (uv_poll_t* watcher, int status, int revents);

}; /* namespace raw */


#endif 