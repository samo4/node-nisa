

#include "makecallback.h"

#include <uv.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <string.h>
#include <ctime>

#include <nan.h>
#include <node.h>

#define MAX_CNT 1024

#ifdef _WIN32
static char errbuf[1024];
#endif
const char* raw_strerror (int code) {
#ifdef _WIN32
	if (FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, errbuf,
			1024, NULL)) {
		return errbuf;
	} else {
		strcpy (errbuf, "Unknown error");
		return errbuf;
	}
#else
	return strerror (code);
#endif
}


namespace raw {
  using namespace v8;
  
  void async_propagate(uv_async_t *async);
  
  static Persistent<FunctionTemplate> _constructor;
  void InitAll (Handle<Object> target) {
    
      VisaEmitter::Init();
      
      Local<Function> constructor = NanNew<FunctionTemplate>(_constructor)->GetFunction();
	    target->Set(NanNew<String>("VisaEmitter"), constructor);
  } 
  
  NODE_MODULE(raw, InitAll)  
  
  typedef struct {
    uint16_t stb;
  } vi_callback_result_t;
  
  //std::set<VisaEmitter const *> VisaEmitter::instances;
  //std::vector<VisaEmitter*> VisaEmitter::instances;
  std::vector<VisaEmitter*> VisaEmitter::instances = std::vector<VisaEmitter*>();
  
  ViStatus write(ViSession instr1, const char* input);
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle); 
  
  VisaEmitter::VisaEmitter() {
    instances.push_back(this); 
    //instances.insert(this);
  }
  
  VisaEmitter::~VisaEmitter() {
    //if (poll_initialised_)
      
    instances[0] = NULL;
    
    //instances.erase(this);
  }
  
  void VisaEmitter::Init() {
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(VisaEmitter::New);
    NanAssignPersistent(_constructor, tpl);
    tpl->SetClassName(NanNew("VisaEmitter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
    NODE_SET_PROTOTYPE_METHOD(tpl, "new", New);
    NODE_SET_PROTOTYPE_METHOD(tpl, "ping", Ping);
  }
  
  NAN_METHOD(VisaEmitter::New) {
    NanScope();
	  VisaEmitter* ve = new VisaEmitter ();
    ve->poll_initialised_ = false;
    int viStatus;
    viStatus = ve->Connect();
    if (viStatus) {
      NanThrowError(raw_strerror (viStatus));
      NanReturnUndefined();
    }
    printf("VisaEmitter::New\n");
    ve->Wrap (args.This ());
	  NanReturnThis();
  }
  
  NAN_METHOD(VisaEmitter::Ping) {
    NanScope();
    Local<Value> argv[2] = {
      NanNew<String>("event"),  // event name
      args[0]
    };
    
    NanMakeCallback(args.This(), "emit", 2, argv);
    printf("let's make an explosion\n");
    
    ViSession defaultRM;
    ViSession instr1;
    ViStatus status;
    ViBuf bufferHandle;
    ViEventType etype;
    ViEvent eventContext;
    
    status = viOpenDefaultRM(&defaultRM);
    status = viOpen(defaultRM, "GPIB0::11::INSTR", VI_NULL, VI_NULL, &instr1);
    viGpibControlREN(instr1, VI_GPIB_REN_ASSERT);
    write(instr1, "X");
    viClear(instr1);
    write(instr1, "M1X"); // enable SRQ on various errors
  
    status = viInstallHandler(instr1, VI_EVENT_SERVICE_REQ, callback, bufferHandle);
	  status = viEnableEvent(instr1, VI_EVENT_SERVICE_REQ, VI_HNDLR, VI_NULL);
  
  
    write(instr1, "D9X");
  
    //NanMakeCallback(args.This(), "emit", 2, argv);
    NanReturnUndefined();
  }
  
  int VisaEmitter::Connect (void) {
    if (this->poll_initialised_)
      return 1;

    m_async = uv_async_t();
    m_async.data = this;    
    uv_async_init(uv_default_loop(), &m_async, reinterpret_cast<uv_async_cb>(aCallback));      
    this->poll_initialised_ = true;
	
    return 0; //OK
  }
  
  void VisaEmitter::HandleIOEvent (int status, int srqStatus) {
    NanScope();
    if (status) {
      Local<Value> emit = NanObjectWrapHandle(this)->Get (NanNew<String>("emit"));
      Local<Function> cb = emit.As<Function> ();
  
      Local<Value> args[2];
      args[0] = NanNew<String>("error");

      char status_str[32];
      sprintf(status_str, "%d", status);
      args[1] = NanError(status_str);
  
      cb->Call (NanObjectWrapHandle(this), 2, args);
    } else {
      Local<Value> emit = NanObjectWrapHandle(this)->Get (NanNew<String>("emit"));
      Local<Function> cb = emit.As<Function> ();
  
      const int argc = 2;
      Local<Value> args[argc];
      args[0] = NanNew<String>("srq");
      args[1] = NanNew<Integer>(srqStatus);
      cb->Call (NanObjectWrapHandle(this), argc, args);
    }
    printf("HandleIOEvent: %d, %d\n", status, srqStatus);
  }
  
  void VisaEmitter::DispatchEventToAllInstances(int stb)
  {
    uv_async_send(&(instances.at(0)->m_async));
    
    /*
    for(auto i : instances) 
    
    static void IoEvent (uv_poll_t* watcher, int status, int revents) {
      VisaEmitter *ve = static_cast<VisaEmitter*>(watcher->data);
      ve->HandleIOEvent (status, revents);
    } */
  }
  
  
  
  void VisaEmitter::aCallback(uv_async_t *handle, int status) {
    if (!handle->data) 
      return;
    
    VisaEmitter* async = static_cast<VisaEmitter*>(handle->data);
    async->HandleIOEvent (0, 12);
    // if we call this, then no more events are 
    // uv_close((uv_handle_t*) async, NULL); ??
    printf("async.HandleIOEvent\n");
    return;
    
    NanScope();
    
  // v8::Local<v8::Value> emit = this->handle()->Get(Nan::New<v8::String>("emit"));
  // v8::Local<v8::Function> cb = emit.As<v8::Function>();
    
    
    //Handle<Object> globalObj = NanGetCurrentContext()->Global();
    //vi_callback_result_t* data = (vi_callback_result_t*) async->data;
    
    /*
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = {
        NanNew<string>("event"),   // event name: change to SRQ
        NanNew(data->stb)				  // result?
    };*/
    //When to use MakeCallBack: https://github.com/nodejs/nan/issues/284
  
    //Isolate* isolate = v8::Isolate::GetCurrent();
    //Local<String> emit_symbol = String::NewFromUtf8(isolate, "emit");
    //Local<Function> cb = globalObj->Get(emit_symbol).As<v8::Function>();
    
    ////v8::Local<v8::Value> argv[argc] = { v8::String::NewFromUtf8(isolate, "hello world") };
    ////cb->Call(globalObj, argc, argv);
    //NanMakeCallback(globalObj, cb, argc, argv);
    
    // perhaps we should call this sooner?
    
  }
  
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle)
  {
    ViJobId jobID;
    ViStatus status;
    ViUInt16 stb;
    status = viReadSTB(vi, &stb);
    if ((status >= VI_SUCCESS) && (stb & 0x40))
    {   
      printf("callback: %d\n", stb);
      VisaEmitter::DispatchEventToAllInstances(stb);
      // we might need uv_mutex_t
      //vi_callback_result_t* data = (vi_callback_result_t*)malloc (sizeof (vi_callback_result_t));
      //data->stb = stb;
      //async->data = (void*) data;
      //async->session = vi; // I think we'll need this later to establish exactly where the event came from.
      //uv_async_send(async);
      
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