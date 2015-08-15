

#include "makecallback.h"

#include <uv.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <string.h>
#include <ctime>

#define MAX_CNT 1024


namespace raw {
  using namespace v8;
  
  static Persistent<FunctionTemplate> _constructor;
  
  void InitAll (Handle<Object> target) {
    
    
    // Prepare constructor template
    /*
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New<v8::String>("VisaEmitter").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
  
    SetPrototypeMethod(tpl, "ping", Ping);
  
    constructor.Reset(tpl->GetFunction());
    Set(target, Nan::New("VisaEmitter").ToLocalChecked(), tpl->GetFunction());*/
    
      VisaEmitter::Init();
    
      Local<Function> constructor = Nan::New<FunctionTemplate>(_constructor)->GetFunction();
      target->Set(Nan::New<String>("VisaEmitter").ToLocalChecked(), constructor);
  } 
  
  NODE_MODULE(raw, InitAll)
  
  

  uv_loop_t *loop;
  uv_async_t* async;
  
  
  typedef struct {
    uint16_t stb;
  } vi_callback_result_t;
  
  
  
  ViStatus write(ViSession instr1, const char* input);
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle);
  
  
   
  
  VisaEmitter::VisaEmitter() {
  }
  
  VisaEmitter::~VisaEmitter() {
  }
  
  void VisaEmitter::Init() {
    Isolate* isolate = Isolate::GetCurrent();
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(VisaEmitter::New);
    _constructor.Reset(isolate, tpl); //NanAssignPersistent(_constructor, tpl);
    
    tpl->SetClassName(Nan::New<String>("VisaEmitter").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    SetPrototypeMethod(tpl, "new", New);
    SetPrototypeMethod(tpl, "ping", Ping); 
  }
  
  NAN_METHOD(VisaEmitter::New) {
    Nan::HandleScope scope;
	  VisaEmitter* ve = new VisaEmitter ();
    ve->poll_initialised_ = false;
    /*
    rc = socket->CreateSocket ();
	if (rc != 0) {
		NanThrowError(raw_strerror (rc));
		NanReturnUndefined();
	}
    */
    ve->Wrap (info.This());
	  //Nan::ReturnValue(info.This()); //Nan::ReturnThis();  ??
    info.GetReturnValue().Set(info.This());
  }
  
  NAN_METHOD(VisaEmitter::Ping) {
      
    Local<Value> argv[2] = {
      Nan::New("event").ToLocalChecked(),  // event name
      info[0]
    };
  
    Nan::MakeCallback(info.This(), "emit", 2, argv);
    
    
    info.GetReturnValue().SetUndefined();
    //return info.GetReturnValue().SetNull();
    //info.GetReturnValue().Set(errorno);
  }
  
  // NODE_MODULE(makecallback, VisaEmitter::Init)
  
  void async_propagate(uv_async_t *async) {
    if (!async->data) 
      return;
    
    Nan::HandleScope scope;
    
  // v8::Local<v8::Value> emit = this->handle()->Get(Nan::New<v8::String>("emit"));
  // v8::Local<v8::Function> cb = emit.As<v8::Function>();
    
    
    v8::Handle<v8::Object> globalObj = Nan::GetCurrentContext()->Global();
    vi_callback_result_t* data = (vi_callback_result_t*) async->data;
    
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = {
        Nan::New("event").ToLocalChecked(),   // event name: change to SRQ
        Nan::New(data->stb)				  // result?
    };
    //When to use MakeCallBack: https://github.com/nodejs/nan/issues/284
  
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::String> emit_symbol = v8::String::NewFromUtf8(isolate, "emit");
    v8::Local<v8::Function> cb = globalObj->Get(emit_symbol).As<v8::Function>();
    //v8::Local<v8::Value> argv[argc] = { v8::String::NewFromUtf8(isolate, "hello world") };
    //cb->Call(globalObj, argc, argv);
    Nan::MakeCallback(globalObj, cb, argc, argv);
    
    // perhaps we should call this sooner?
    uv_close((uv_handle_t*) async, NULL);
  }
  
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle)
  {
    ViJobId jobID;
    ViStatus status;
    ViUInt16 stb;
    status = viReadSTB(vi, &stb);
    if ((status >= VI_SUCCESS) && (stb & 0x40))
    {   
      // we might need uv_mutex_t
      vi_callback_result_t* data = (vi_callback_result_t*)malloc (sizeof (vi_callback_result_t));
      data->stb = stb;
      async->data = (void*) data;
      //async->session = vi; // I think we'll need this later to establish exactly where the event came from.
      uv_async_send(async);
      
      // async.data = stb; who cares about the stb.. you can read it later.
      //uv_async_send(&async);	
      // printf("SQR :0x%02x\n", stb); /// yes, it's mine :-) 
    }
    return VI_SUCCESS;
  }
  
  
  ViStatus write(ViSession instr1, const char* input)
  {
    ViUInt32 writeCount;
    char temp[100];
    _snprintf_s(temp, sizeof(temp), input);
    return viWrite(instr1, (ViBuf)temp, (ViUInt32)strlen(temp), &writeCount);
  }
}