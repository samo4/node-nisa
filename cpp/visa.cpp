#include "makecallback.h"
#include <uv.h>

namespace raw {
	
	
	void VisaEmitter::StaticWrite(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_Write(data);
	}
	
	void VisaEmitter::EIO_Write(QueuedWrite* queuedWrite) {
		WriteBaton* data = static_cast<WriteBaton*>(queuedWrite->baton);
    
		if (!sizeof(data->command) || !this->isConnected)
			return;
		
		char temp[QUERY_STRING_SIZE];
		memset(temp, 0, sizeof(temp));
		ViStatus status;
		if (session < 1) {
			ErrorCodeToString("not open", 11, data->errorString);
			return;
		}
		
		ViInt32 rdBufferSize = sizeof(temp);
		ViUInt32 returnCount;
		
		status = viWrite(session, (ViBuf)data->command, strlen(data->command), &returnCount);
		if ((status < VI_SUCCESS)) {
			_snprintf(temp, sizeof(temp), "%d viWrite, query: %s string length: %d", session, data->command, strlen(data->command));
			ErrorCodeToString(temp, status, data->errorString);
			return;
		}

		data->result[0] = 0;
	}
	
	void VisaEmitter::EIO_AfterWrite(uv_work_t* req) {
		NanScope();
		QueuedWrite* queuedWrite = static_cast<QueuedWrite*>(req->data);
		WriteBaton* baton = static_cast<WriteBaton*>(queuedWrite->baton);
		
		Handle<Value> argv[2];
		if(baton->errorString[0]) {
			argv[0] = Exception::Error(NanNew<String>(baton->errorString));
			argv[1] = NanUndefined();
		} else {
			argv[0] = NanUndefined();
			argv[1] = NanNew(baton->result);
		}
		baton->callback->Call(2, argv);
		
		delete baton->callback;
  		delete baton;
	}
	
	NAN_METHOD(VisaEmitter::Ping) {
		NanScope();
		Local<Value> argv[2] = {
			NanNew<String>("ping"),  // event name
			args[0]
		};
		
		NanMakeCallback(args.This(), "emit", 2, argv);
		//printf("let's make an explosion\n");
		/*
		write(instr1, "D9X");*/
	
		//NanMakeCallback(args.This(), "emit", 2, argv);
		NanReturnUndefined();
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
  
  ViStatus write(ViSession session, const char* input)
  {
    ViUInt32 writeCount;
    if (sizeof(input) > 4) {
      throw std::invalid_argument( "viWrite input string too long" );
    }
    
    char temp[500];
    _snprintf_s(temp, sizeof(temp), input);
    return viWrite(session, (ViBuf)temp, (ViUInt32)strlen(temp), &writeCount);
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
      raw::VisaEmitter::DispatchEventToAllInstances(stb);
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
  
  void ErrorCodeToString(const char* prefix, int errorCode, char *errorStr) {
    switch(errorCode) {
    // success codes for completeness
    case VI_SUCCESS:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Specified access mode is successfully acquired. ", prefix);
    break;
    case VI_SUCCESS_NESTED_EXCLUSIVE:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Specified access mode is successfully acquired, and this session has nested exclusive locks. ", prefix);
    break;
    case VI_SUCCESS_NESTED_SHARED:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Specified access mode is successfully acquired, and this session has nested shared locks. ", prefix);
    break;
    // Error codes:
    case VI_ERROR_INV_OBJECT:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | The given session or object reference is invalid . ", prefix);
    break;
    case VI_ERROR_RSRC_LOCKED:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Specified type of lock cannot be obtained because the resource is already locked with a lock type incompatible with the lock requested. ", prefix);
    break;
    case VI_ERROR_INV_LOCK_TYPE:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | The specified type of lock is not supported by this resource. ", prefix);
    break;
    case VI_ERROR_INV_ACCESS_KEY:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | The requestedKey value passed in is not a valid access key to the specified resource. ", prefix);
    break;
    case VI_ERROR_TMO:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Timeout error.", prefix);
    break;
    default:
      _snprintf(errorStr, ERROR_STRING_SIZE, "%s | Unknown error code %d", prefix, errorCode);
      break;
    }
  }
}