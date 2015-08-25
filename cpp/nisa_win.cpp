#include "nisa.h"
#include <uv.h>

namespace raw {
	
  /* OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN OPEN */
  
  void VisaEmitter::EIO_Open(QueuedWrite* queuedWrite) {
		GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
      
    char temp[QUERY_STRING_SIZE];
    ViStatus status = -1;
    if (this->isConnected)
    {
      _snprintf(temp, sizeof(temp), "Already connected %s", session);
      ErrorCodeToString(temp, status, data->errorString);
      return;
    }
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS) {
      _snprintf(temp, sizeof(temp), "Opening RM");
      ErrorCodeToString(temp, status, data->errorString);
      return;
    }
    status = viOpen(defaultRM, data->command, VI_NULL, VI_NULL, &session);
    if (status < VI_SUCCESS) {
      _snprintf(temp, sizeof(temp), "Opening session %s", data->command);
      ErrorCodeToString(temp, status, data->errorString);
      return;
    }
    status = viSetAttribute(session, VI_ATTR_TMO_VALUE, 5000);
    if (status < VI_SUCCESS) {
      _snprintf(temp, sizeof(temp), "Setting attributes on %s", data->command);
      ErrorCodeToString(temp, status, data->errorString);
      return;
    }
    
    this->isConnected = true;
    // status = viSetAttribute(instr, VI_ATTR_SEND_END_EN, VI_TRUE);
    // terminate reads on a carriage return  0x0a 0x0d
    // LF (Line feed, '\n', 0x0A, 10 in decimal) 
    // Carriage return, '\r', 0x0D, 13 in decimal
    
    // status = viSetAttribute(session, VI_ATTR_TERMCHAR, 0x0A);
    //status = viSetAttribute(session, VI_ATTR_TERMCHAR_EN, VI_TRUE);
    
    if (data->assertREN) {
      viGpibControlREN(session, VI_GPIB_REN_ASSERT);
    }
    
    if (data->enableSRQ)  
    {
      m_async = uv_async_t();
      m_async.data = this;    
      uv_async_init(uv_default_loop(), &m_async, reinterpret_cast<uv_async_cb>(aCallback));
      
      ViBuf bufferHandle = (ViBuf)malloc(ERROR_STRING_SIZE);
      status = viInstallHandler(session, VI_EVENT_SERVICE_REQ, callback, bufferHandle);
      if (status >= VI_SUCCESS) {
        status = viEnableEvent(session, VI_EVENT_SERVICE_REQ, VI_HNDLR, VI_NULL);
      }  
      if (status < VI_SUCCESS) {
        _snprintf(temp, sizeof(temp), "Post AfterOpenSuccess session %s", data->command);
        ErrorCodeToString(temp, status, data->errorString);
        return;
      }        
    }    
      
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%d", session);
  }
  
	/* WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE WRITE  */
	
	void VisaEmitter::EIO_Write(QueuedWrite* queuedWrite) {
		GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
		if (!sizeof(data->command) || !this->isConnected || session < 1) {
			ErrorCodeToString("not connected or bad empty command", 11, data->errorString);
			return;
		}
      
		char temp[QUERY_STRING_SIZE];
		memset(temp, 0, sizeof(temp));
		ViInt32 rdBufferSize = sizeof(temp);
		ViUInt32 returnCount;
		ViStatus status = viWrite(session, (ViBuf)data->command, (ViUInt32) strlen(data->command), &returnCount);
		if ((status < VI_SUCCESS)) {
			_snprintf(temp, sizeof(temp), "%d viWrite, query: %s string length: %d; wrote: %d", session, data->command, strlen(data->command), returnCount);
			ErrorCodeToString(temp, status, data->errorString);
			return;
		}
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%d", returnCount); 
	}
  
  /* READ READ READ */
  
  void VisaEmitter::EIO_Read(QueuedWrite* queuedWrite) {
		GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
		if (!this->isConnected || session < 1) {
			ErrorCodeToString("not connected", 11, data->errorString);
			return;
		}
      
		char temp[QUERY_STRING_SIZE];
		memset(temp, 0, sizeof(temp));
		ViInt32 rdBufferSize = sizeof(temp);
		ViUInt32 returnCount;
    
    ViStatus status = viRead(session, (ViBuf)temp, QUERY_STRING_SIZE, &returnCount);
		if ((status < VI_SUCCESS)) {
			_snprintf(temp, sizeof(temp), "%d viRead, returnCount: %d", session, returnCount);
			ErrorCodeToString(temp, status, data->errorString);
			return;
		}
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%s", temp);
	}  
  
  /* QUERY QUERY */
  
  void VisaEmitter::EIO_Query(QueuedWrite* queuedWrite) {
    GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
    if (!sizeof(data->command) || !this->isConnected || session < 1) {
			ErrorCodeToString("not connected or bad empty command", 11, data->errorString);
			return;
		}
    char temp[QUERY_STRING_SIZE];
    ViChar rdBuffer[256];
    ViStatus status = viQueryf(session, (ViString)data->command, "%256[^,]%*T", rdBuffer);
    if ((status < VI_SUCCESS)) {
      _snprintf(temp, sizeof(temp), "%d viQueryf, query: %s string length: %d", session, data->command, strlen(data->command));
      ErrorCodeToString(temp, status, data->errorString);
      return;
    }
    // this will not go down nicely on non-WIN32...
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%s", rdBuffer );  
    return;
  }
  
  /* TRIGGER TRIGGER */
  
  void VisaEmitter::EIO_Trigger(QueuedWrite* queuedWrite) {
    GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
    if (!this->isConnected || session < 1) {
			ErrorCodeToString("not connected", 11, data->errorString);
      printf("not connected");
			return;
		}
    char temp[QUERY_STRING_SIZE];
    ViStatus status = viAssertTrigger(session, VI_TRIG_PROT_DEFAULT);
    if ((status < VI_SUCCESS)) {
      _snprintf(temp, sizeof(temp), "%d viAssertTrigger", session);
      printf("status < VI_SUCCESS");
      ErrorCodeToString(temp, status, data->errorString);
      return;
    } 
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%d", 0);
  }
  
  /* device clear */
  
  void VisaEmitter::EIO_DeviceClear(QueuedWrite* queuedWrite) {
    
    GenericBaton* data = static_cast<GenericBaton*>(queuedWrite->baton);
    if (!this->isConnected || session < 1) {
			ErrorCodeToString("not connected", 11, data->errorString);
			return;
		}
    
    char temp[QUERY_STRING_SIZE];
    ViStatus status = viClear(session);
    
    if ((status < VI_SUCCESS)) {
      _snprintf(temp, sizeof(temp), "%d viClear", session);
      printf("status < VI_SUCCESS");
      ErrorCodeToString(temp, status, data->errorString);
      return;
    } 
    _snprintf_s(data->result, _countof(data->result), _TRUNCATE, "%d", 0);
  }
  
  
  
  /* after all after all after all after all after all after all after all after all after all after all after all after all after all after all */
  
  void VisaEmitter::EIO_AfterAll(uv_work_t* req) {
    NanScope();
		QueuedWrite* queuedWrite = static_cast<QueuedWrite*>(req->data);
		GenericBaton* baton = static_cast<GenericBaton*>(queuedWrite->baton);
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
    delete queuedWrite;
	}
	
  /* CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK  */
	
	void VisaEmitter::aCallback(uv_async_t *handle, int status) {
    if (!handle->data) 
      return;
    
    VisaEmitter* async = static_cast<VisaEmitter*>(handle->data);
    async->HandleHardwareEvent (0, async->lastSTB);
    // if we call this, then no more events are 
    // uv_close((uv_handle_t*) async, NULL); ??
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
  
  ViStatus _VI_FUNCH callback(ViSession session_, ViEventType etype, ViEvent eventContext, ViAddr userHandle)
  {
    ViStatus status;
    ViUInt16 stb;
    status = viReadSTB(session_, &stb);
    if ((status >= VI_SUCCESS) && (stb & 0x40))
    {   
      raw::VisaEmitter::DispatchEventToAllInstances(stb, session_);
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
  
  /* LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL LOW LEVEL */
  
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