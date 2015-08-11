#include <nan.h>

#include <visa.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <string.h>
#include <ctime>

#define MAX_CNT 1024

ViStatus write(ViSession instr1, const char* input);
ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle);

using namespace Nan;  // NOLINT(build/namespaces)

class MyObject : public node::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);

 private:
  MyObject();
  ~MyObject();
  static NAN_METHOD(New);
  static NAN_METHOD(CallEmit);
  static Persistent<v8::Function> constructor;
  void MyObject::ConnectToDamnVIsa();
  
};

Persistent<v8::Function> MyObject::constructor;

MyObject::MyObject() {
}

MyObject::~MyObject() {
}

NAN_MODULE_INIT(MyObject::Init) {
  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<v8::String>("MyObject").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "call_emit", CallEmit);

  constructor.Reset(tpl->GetFunction());
  Set(target, Nan::New("MyObject").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(MyObject::New) {
  if (info.IsConstructCall()) {
    MyObject* obj = new MyObject();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance());
  }
}

NAN_METHOD(MyObject::CallEmit) {
  v8::Local<v8::Value> argv[1] = {
    Nan::New("event").ToLocalChecked(),  // event name
  };

  MakeCallback(info.This(), "emit", 1, argv);
  info.GetReturnValue().SetUndefined();
}

NODE_MODULE(makecallback, MyObject::Init)



/*********************************/

void MyObject::ConnectToDamnVIsa()
{
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
  
}


ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle)
{
  v8::Handle<v8::Object> globalObj = Nan::GetCurrentContext()->Global();
  
	ViJobId jobID;
	ViStatus status;
	ViUInt16 stb;
	status = viReadSTB(vi, &stb);
	if ((status >= VI_SUCCESS) && (stb & 0x40))
	{
    
    v8::Local<v8::Value> argv[1] = {
      Nan::New("event").ToLocalChecked(),  // event name
    };
    MakeCallback(Nan::New<v8::Object>(this->This), "emit", 1, argv);
		// printf("SQR :0x%02x\n", stb); /// yes, it's mine :-) 
	}
	status = viReadAsync(vi, (ViBuf)userHandle, MAX_CNT, &jobID);
	return VI_SUCCESS;
}


ViStatus write(ViSession instr1, const char* input)
{
	ViUInt32 writeCount;
	char temp[100];
	_snprintf_s(temp, sizeof(temp), input);
	return viWrite(instr1, (ViBuf)temp, (ViUInt32)strlen(temp), &writeCount);
}