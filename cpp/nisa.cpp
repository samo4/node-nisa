#include "nisa.h"
#include <uv.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <conio.h>

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

  ViSession VisaEmitter::defaultRM;
  static Persistent<FunctionTemplate> _constructor;
  std::vector<VisaEmitter*> VisaEmitter::instances = std::vector<VisaEmitter*>();

  /*  typedef struct {
    uint16_t stb;
  } vi_callback_result_t; */
  
  void InitAll (Handle<Object> target) {
    
      VisaEmitter::Init();
      
      Local<Function> constructor = NanNew<FunctionTemplate>(_constructor)->GetFunction();
	    target->Set(NanNew<String>("VisaEmitter"), constructor);
  } 
  
  NODE_MODULE(raw, InitAll)  
  
  VisaEmitter::VisaEmitter(std::string s) {
    address_ = new std::string(s);
    instances.push_back(this); 
    this->uniqueSRQhandlerIdentification = (ViAddr) instances.size();
  }
  
  VisaEmitter::~VisaEmitter() {
    delete address_;
    if (isConnected)
      viClose(session);
      
    printf("DEBUG: VisaEmitter Destructor");
    
    auto iter = std::find(instances.begin(), instances.end(), this);
    if (iter != instances.end())
    {
        printf("DEBUG: VisaEmitter Deleted something");
        instances.erase(iter);
    }
  }
  
  void VisaEmitter::Init() {
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(VisaEmitter::New);
    NanAssignPersistent(_constructor, tpl);
    tpl->SetClassName(NanNew("VisaEmitter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
    NODE_SET_PROTOTYPE_METHOD(tpl, "new", New);
    NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write", Write);
    NODE_SET_PROTOTYPE_METHOD(tpl, "read", Read);
    NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);
    NODE_SET_PROTOTYPE_METHOD(tpl, "trigger", Trigger);
    NODE_SET_PROTOTYPE_METHOD(tpl, "deviceClear", DeviceClear);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  }
  
  NAN_METHOD(VisaEmitter::New) {
    NanScope();
    if(!args[0]->IsString()) {
      NanThrowTypeError("First argument must be a string with VISA address. Something like: GPIB0::11::INSTR");
      NanReturnUndefined();
    }
    String::Utf8Value path(args[0]->ToString());
  
    if (args.IsConstructCall()) {
      String::Utf8Value str(args[0]->ToString());
      std::string s(*str);
      VisaEmitter* ve = new VisaEmitter(s);
      ve->isConnected = false;
      ve->Wrap(args.This());
      //return args.This(); //return scope.Close(args.This());
      NanReturnThis();
    } else {
      NanThrowTypeError("not sure if I know why this is good.. best if we throw exception for the moment" );
      /*const int argc = 2;
      Handle<Value> argv[argc];
      argv[0] = args[0]->ToString();
      argv[1] = args[1]->ToObject();
      return scope.Close(constructor->NewInstance(argc, argv));*/
    }   
  }
  
  NAN_METHOD(VisaEmitter::Open) {
    NanScope();
    VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsObject()) {
      NanThrowTypeError("First argument must be an options object.");
      NanReturnUndefined();
    }
    Local<Object> options = args[0]->ToObject();
    
    if(!args[1]->IsFunction()) {
      NanThrowTypeError("Second argument must be a function (err, res)");
      NanReturnUndefined();
    }
    Local<Function> callback = args[1].As<Function>();
    
    GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
    strcpy(baton->command, obj->address_->c_str());
		baton->callback = new NanCallback(callback);
    baton->enableSRQ = options->Get(NanNew<String>("enableSRQ"))->ToBoolean()->BooleanValue();
    baton->assertREN = options->Get(NanNew<String>("assertREN"))->ToBoolean()->BooleanValue();
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req, VisaEmitter::StaticOpen, (uv_after_work_cb)VisaEmitter::EIO_AfterAll);    
    NanReturnUndefined();
  }
  
  NAN_METHOD(VisaEmitter::Write) {
		NanScope();
		VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
		
		if(!args[0]->IsString()) {
			NanThrowTypeError("First argument must be a string");
			NanReturnUndefined();
		}
		String::Utf8Value cmd(args[0]->ToString());
		
		if(!args[1]->IsFunction()) {
			NanThrowTypeError("Second argument must be a function: err, res");
			NanReturnUndefined();
		}
		Local<Function> callback = args[1].As<Function>();
		
		GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
		strcpy(baton->command, *cmd);
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req, VisaEmitter::StaticWrite, (uv_after_work_cb)VisaEmitter::EIO_AfterAll);
		NanReturnUndefined();
	}
  
  NAN_METHOD(VisaEmitter::Read) {
		NanScope();
		VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsNumber()) {
			NanThrowTypeError("First argument must be a number");
			NanReturnUndefined();
		}
    int64_t numberOfBytes = args[0]->NumberValue();
    
		if(!args[1]->IsFunction()) {
			NanThrowTypeError("second argument must be a function: err, res");
			NanReturnUndefined();
		}
		Local<Function> callback = args[1].As<Function>();
		
		GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
    _snprintf_s(baton->command, _countof(baton->command), _TRUNCATE, "%d", numberOfBytes);
		strcpy(baton->errorString, "");
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req, VisaEmitter::StaticRead, (uv_after_work_cb)VisaEmitter::EIO_AfterAll);
		NanReturnUndefined();
	}
  
  NAN_METHOD(VisaEmitter::Query) {
    NanScope();
    VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsString()) {
      NanThrowTypeError("First argument must be a string");
      NanReturnUndefined();
    }
    String::Utf8Value cmd(args[0]->ToString());
    
    if(!args[1]->IsFunction()) {
      NanThrowTypeError("Second argument must be a function: err, res");
      NanReturnUndefined();
    }
    Local<Function> callback = args[1].As<Function>();
    
    GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
		strcpy(baton->command, *cmd);
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req, VisaEmitter::StaticQuery, (uv_after_work_cb)VisaEmitter::EIO_AfterAll);
    NanReturnUndefined();
  }
  
  NAN_METHOD(VisaEmitter::Trigger) {
    NanScope();
    VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsFunction()) {
      NanThrowTypeError("Argument must be a function");
      NanReturnUndefined();
    }
    Local<Function> callback = args[0].As<Function>();
    
    GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
    strcpy(baton->command, obj->address_->c_str());
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req, VisaEmitter::StaticTrigger, NULL);
  }
  
  NAN_METHOD(VisaEmitter::DeviceClear) {
    NanScope();
    VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsFunction()) {
      NanThrowTypeError("Argument must be a function");
      NanReturnUndefined();
    }
    Local<Function> callback = args[0].As<Function>();
    
    GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
    strcpy(baton->command, "");
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req,  VisaEmitter::StaticDeviceClear,  (uv_after_work_cb)VisaEmitter::EIO_AfterAll);
  }
  
  NAN_METHOD(VisaEmitter::Close) {
    NanScope();
    VisaEmitter* obj = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsFunction()) {
      NanThrowTypeError("Argument must be a function");
      NanReturnUndefined();
    }
    Local<Function> callback = args[0].As<Function>();
    
    GenericBaton* baton = new GenericBaton();
		memset(baton, 0, sizeof(GenericBaton));
		strcpy(baton->errorString, "");
    strcpy(baton->command, "");
		baton->callback = new NanCallback(callback);
    baton->obj = obj;
    uv_work_t *req = new uv_work_t;
    baton->req = *req;
    baton->req.data = baton; 
    uv_queue_work(uv_default_loop(), &baton->req,  VisaEmitter::StaticClose,  (uv_after_work_cb)VisaEmitter::EIO_AfterAll);
  }
  
  void VisaEmitter::HandleHardwareEvent (int status, int srqStatus) {
    NanScope();
    /*if (status) {
      Local<Value> emit = NanObjectWrapHandle(this)->Get (NanNew<String>("emit"));
      Local<Function> cb = emit.As<Function> ();
  
      Local<Value> args[2];
      args[0] = NanNew<String>("error");

      char status_str[32];
      sprintf(status_str, "%d", status);
      args[1] = NanError(status_str);
  
      cb->Call (NanObjectWrapHandle(this), 2, args);
    } else {*/
      Local<Value> emit = NanObjectWrapHandle(this)->Get (NanNew<String>("emit"));
      Local<Function> cb = emit.As<Function> ();
  
      const int argc = 2;
      Local<Value> args[argc];
      args[0] = NanNew<String>("srq");
      args[1] = NanNew<Integer>(srqStatus);
      cb->Call (NanObjectWrapHandle(this), argc, args);
    //}
  }
  
  void VisaEmitter::DispatchEventToAllInstances(int stb, ViSession session_)
  {
     // uv_async_send(&(instances.at(0)->m_async)); 
    for(auto i : instances) 
    {
      if (i->session == session_){
        i->lastSTB = stb; 
        uv_async_send(&(i->m_async));  
      }
    }
    /* static void IoEvent (uv_poll_t* watcher, int status, int revents) {
      VisaEmitter *ve = static_cast<VisaEmitter*>(watcher->data);
      ve->HandleHardwareEvent (status, revents);
    } */
  }
}