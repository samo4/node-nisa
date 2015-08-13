#ifndef _H_MAKE_CALLBACK_
#define _H_MAKE_CALLBACK_

#include <nan.h>

class VisaEmitter : public node::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);
  static void async_propagate(uv_async_t* async);
 private:
  VisaEmitter();
  ~VisaEmitter();
  static NAN_METHOD(New);
  static NAN_METHOD(Ping);
  static Nan::Persistent<v8::Function> constructor;
};

#endif 