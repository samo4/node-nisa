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
  
  uv_mutex_t write_queue_mutex;
  QUEUE write_queue;

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
	  //uv_async_init(uv_default_loop(), &async_, print_progress);	
	  //this->poll_initialised_ = true;
    instances.push_back(this); 
  }
  
  VisaEmitter::~VisaEmitter() {
    delete address_;
    if (isConnected)
      viClose(session);
      
    instances[0] = NULL;
    //instances.erase(this);
  }
  
  void VisaEmitter::Init() {
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(VisaEmitter::New);
    NanAssignPersistent(_constructor, tpl);
    tpl->SetClassName(NanNew("VisaEmitter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
    NODE_SET_PROTOTYPE_METHOD(tpl, "new", New);
    NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write", Write);
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
      throw std::invalid_argument( "not sure if I know why this is good.. best if we throw exception for the moment" );
      /*const int argc = 2;
      Handle<Value> argv[argc];
      argv[0] = args[0]->ToString();
      argv[1] = args[1]->ToObject();
      return scope.Close(constructor->NewInstance(argc, argv));*/
    }   
  }
  
  NAN_METHOD(VisaEmitter::Open) {
    NanScope();
    VisaEmitter* ve = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    uv_mutex_init(&write_queue_mutex);
    QUEUE_INIT(&write_queue);
    
    if(!args[0]->IsFunction()) {
      NanThrowTypeError("Argument must be a function");
      NanReturnUndefined();
    }
    Local<Function> callback = args[0].As<Function>();
    
    OpenBaton* baton = new OpenBaton();
    memset(baton, 0, sizeof(OpenBaton));
    strcpy(baton->path, ve->address_->c_str());
    baton->callback = new NanCallback(callback);
    // baton->instr = &obj->instr;
  
    uv_work_t* req = new uv_work_t();
    baton->obj = ve;
    req->data = baton;
    uv_queue_work(uv_default_loop(), req, VisaEmitter::StaticOpen, (uv_after_work_cb)VisaEmitter::EIO_AfterOpen);
    
    NanReturnUndefined();
  }
  
  /*
  NAN_METHOD(VisaEmitter::Open) {
    NanScope();
    VisaEmitter* ve = ObjectWrap::Unwrap<VisaEmitter>(args.This());
    
    if(!args[0]->IsFunction()) {
      NanThrowTypeError("Argument must be a function(err, res)");
      NanReturnUndefined();
    }
    Local<Function> callback = args[0].As<Function>();
    
    Handle<Value> argv[2];
    int viStatus = ve->Connect();
    if (viStatus != VI_SUCCESS) {
      argv[0] = v8::Exception::Error(NanNew<v8::String>(raw_strerror (viStatus)));
      argv[1] = NanUndefined();
    }
    else {
      argv[0] = NanUndefined();
      argv[1] = NanNew<v8::Int32>(0);
    }
    
    // someday, this should be async.. u
    (new NanCallback(callback))->Call(2, argv);
    
    if (viStatus == VI_SUCCESS) {
      Handle<Value> argv[2] = {
        NanNew<v8::String>("open"), 
        NanNew<v8::Int32>(0)
      };
      NanMakeCallback(args.This(), "emit", 2, argv);
    }
  } */
  
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
		
		WriteBaton* baton = new WriteBaton();
		memset(baton, 0, sizeof(WriteBaton));
		strcpy(baton->errorString, "");
		strcpy(baton->command, *cmd);
		baton->callback = new NanCallback(callback);
		
		uv_work_t* req = new uv_work_t();
  	req->data = baton;
  
		QueuedWrite* queuedWrite = new QueuedWrite();
		memset(queuedWrite, 0, sizeof(QueuedWrite));
		//QUEUE_INIT(&queuedWrite->queue);
		queuedWrite->baton = baton;
		queuedWrite->req.data = queuedWrite;
		queuedWrite->obj = obj;

		//uv_mutex_lock(&write_queue_mutex);
		//bool empty = QUEUE_EMPTY(&write_queue);
		//QUEUE_INSERT_TAIL(&write_queue, &queuedWrite->queue);
		//if (empty) {
			uv_queue_work(uv_default_loop(), &queuedWrite->req, VisaEmitter::StaticWrite, (uv_after_work_cb)VisaEmitter::EIO_AfterWrite);
		//}
		//uv_mutex_unlock(&write_queue_mutex);
		
		NanReturnUndefined();
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
  
  
  
  
  
  
}