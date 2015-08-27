#ifndef _H_NISA_
#define _H_NISA_

// source: https://bitbucket.org/stephenwvickers/node-raw-socket

// another potential: https://gitlab.com/no9/udt4/tree/master/src
// and another: https://github.com/bramp/nodewii/blob/master/src/wiimote.cc
// more aysnc: http://blog.trevnorris.com/2013/07/node-with-threads.html http://bramp.github.io/nodewii-talk
// and: http://blog.trevnorris.com/2013/07/node-with-threads.html

// and this one when we're ready for 2.x https://github.com/nodejs/nan/issues/376#issuecomment-120838432

#include <nan.h>
#include <visa.h>
//#include <set>
#include <vector>
#include <list>

#define MAX_INSTANCES 50
#define ERROR_STRING_SIZE 1024
#define QUERY_STRING_SIZE 40240

using namespace v8;

namespace raw {
  struct ListBaton;
  struct GenericBaton;
  
  ViStatus write(ViSession instr1, const char* input);
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle);
  void ErrorCodeToString(const char* prefix, int errorCode, char *errorStr);
  
  class VisaEmitter : public node::ObjectWrap {
    public:
      static ViSession defaultRM;
      ViSession session;
      ViAddr uniqueSRQhandlerIdentification;
      int lastSTB;
      
      void HandleHardwareEvent (int status, int revents);
      static void Init();
      static void DispatchEventToAllInstances(int stb, ViSession session_);
    private:
    
      explicit VisaEmitter  (std::string s = "");
      ~VisaEmitter();
      
      static std::vector<VisaEmitter*> instances;
      std::string *address_;
      bool installedSRQHanlder;
    
      static NAN_METHOD(New);
      static NAN_METHOD(Open);
      static NAN_METHOD(Write);
      static NAN_METHOD(Read);
      static NAN_METHOD(Query);
      static NAN_METHOD(Trigger);
      static NAN_METHOD(DeviceClear);
      static NAN_METHOD(Close);
      
      uv_async_t m_async;
      
      bool isConnected;
      bool isAsyncInitialized;
      
      static void aCallback(uv_async_t *async, int status);
      
      static void StaticOpen(uv_work_t* req);
      void EIO_Open(GenericBaton* data);
      
      static void StaticWrite(uv_work_t* req);
      void VisaEmitter::EIO_Write(GenericBaton* data);
      
      static void StaticRead(uv_work_t* req);
      void EIO_Read(GenericBaton* data);
      
      static void StaticQuery(uv_work_t* req);
      void EIO_Query(GenericBaton* data);
      
      static void StaticTrigger(uv_work_t* req);
      void EIO_Trigger(GenericBaton* data);
      
      static void StaticDeviceClear(uv_work_t* req);
      void EIO_DeviceClear(GenericBaton* data);
      
      static void StaticClose(uv_work_t* req);
      void EIO_Close(GenericBaton* data);
      
      static void EIO_AfterAll(uv_work_t* req);
  }; 
  
  struct ListResultItem {
    public:
      std::string path;
      std::string idn;
  };
  
  struct GenericBaton {
    public:
      uv_work_t req;
      NanCallback* callback;
      VisaEmitter* obj;
      
      char command[QUERY_STRING_SIZE];
      char result[QUERY_STRING_SIZE];
      char errorString[ERROR_STRING_SIZE];
      // properties below are used only for Open 
      bool enableSRQ;
      bool assertREN;
  };
  
  struct ListBaton {
    public:
      NanCallback* callback;
      std::list<ListResultItem*> results;
      char errorString[ERROR_STRING_SIZE];
      VisaEmitter* obj;
  };
}; /* namespace raw */


#endif 