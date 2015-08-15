#ifndef _H_MAKE_CALLBACK_
#define _H_MAKE_CALLBACK_

// source: https://bitbucket.org/stephenwvickers/node-raw-socket

#include <nan.h>
#include <visa.h>

using namespace v8;

namespace raw {

// rename to VisaEmitterWrap?
  class VisaEmitter : public node::ObjectWrap {
    public:
      void HandleIOEvent (int status, int revents);
      static void Init ();
    private:
      VisaEmitter();
      ~VisaEmitter();
      
      static NAN_MODULE_INIT(Init);
    
      static NAN_METHOD(New);
      static NAN_METHOD(Ping);
      
      uv_poll_t *poll_watcher_;
      bool poll_initialised_;
  }; 

  static void IoEvent (uv_poll_t* watcher, int status, int revents);

}; /* namespace raw */


#endif 