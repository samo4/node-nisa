

#include "makecallback.h"

#include <uv.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <string.h>
#include <ctime>

#define MAX_CNT 1024


uv_loop_t *loop;
uv_async_t* async;


typedef struct {
  uint16_t stb;
} vi_callback_result_t;



ViStatus write(ViSession instr1, const char* input);
ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle);


using namespace Nan;  // NOLINT(build/namespaces)

VisaEmitter::VisaEmitter() {
}

VisaEmitter::~VisaEmitter() {
}

NAN_MODULE_INIT(VisaEmitter::Init) {
  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<v8::String>("VisaEmitter").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "ping", Ping);

  constructor.Reset(tpl->GetFunction());
  Set(target, Nan::New("VisaEmitter").ToLocalChecked(), tpl->GetFunction());
} 

Persistent<v8::Function> VisaEmitter::constructor;

NAN_METHOD(VisaEmitter::New) {
  ViSession defaultRM;
  ViSession instr1;
  ViStatus status;
  ViBuf bufferHandle;
  
  if (info.IsConstructCall()) {
    VisaEmitter* obj = new VisaEmitter();
    obj->Wrap(info.This());
      
      //Persistent<v8::Object>::New(info.This());
    
    // begin with hack test code
    status = viOpenDefaultRM(&defaultRM);
    status = viOpen(defaultRM, "GPIB0::11::INSTR", VI_NULL, VI_NULL, &instr1);
    viGpibControlREN(instr1, VI_GPIB_REN_ASSERT);
    write(instr1, "X");
    viClear(instr1);
    write(instr1, "M1X"); // enable SRQ on various errors
    status = viInstallHandler(instr1, VI_EVENT_SERVICE_REQ, callback, bufferHandle);
    status = viEnableEvent(instr1, VI_EVENT_SERVICE_REQ, VI_HNDLR, VI_NULL);
    // end hack test code
    async = new uv_async_t; //async = (uv_async_t*) malloc(sizeof(*async));
    async->data = (void*) this;  //async->data = (void*) info.This(); //(void*) this;
    uv_async_init(uv_default_loop(), async, async_propagate);
    
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance());
  }
}

NAN_METHOD(VisaEmitter::Ping) {
  v8::Local<v8::Value> argv[1] = {
    Nan::New("event").ToLocalChecked(),  // event name
  };
  
  
  ViSession defaultRM;
  ViSession instr1;
  ViStatus status;
  status = viOpenDefaultRM(&defaultRM);
  status = viOpen(defaultRM, "GPIB0::11::INSTR", VI_NULL, VI_NULL, &instr1);
  write(instr1, "R9F0X"); 
   
  // MakeCallback(info.This(), "emit", 1, argv);
  info.GetReturnValue().SetUndefined();
}

NODE_MODULE(makecallback, VisaEmitter::Init)

void VisaEmitter::async_propagate(uv_async_t *async) {
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
  MakeCallback(globalObj, cb, argc, argv);
  
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