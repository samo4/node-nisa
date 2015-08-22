#ifndef _H_MAKE_CALLBACK_
#define _H_MAKE_CALLBACK_

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

#include "queue.h"

#define MAX_INSTANCES 50
#define ERROR_STRING_SIZE 1024
#define QUERY_STRING_SIZE 40240

using namespace v8;



namespace raw {
  struct ListBaton;
  struct OpenBaton;
  struct GenericBaton;
  struct QueuedWrite;
  
  ViStatus write(ViSession instr1, const char* input);
  ViStatus _VI_FUNCH callback(ViSession vi, ViEventType etype, ViEvent eventContext, ViAddr userHandle);
  void ErrorCodeToString(const char* prefix, int errorCode, char *errorStr);
  
  class VisaEmitter : public node::ObjectWrap {
    public:
      static ViSession defaultRM;
      ViSession session;
      int lastSTB;
      
      void HandleHardwareEvent (int status, int revents);
      static void Init();
      static void DispatchEventToAllInstances(int stb, ViSession session_);
    private:
    
      explicit VisaEmitter  (std::string s = "");
      ~VisaEmitter();
      
      static std::vector<VisaEmitter*> instances;
      std::string *address_;
    
      static NAN_METHOD(New);
      static NAN_METHOD(Open);
      static NAN_METHOD(Write);
      static NAN_METHOD(Query);
      
      uv_async_t m_async;
      
      bool isConnected;
      
      static void aCallback(uv_async_t *async, int status);
      
      static void StaticOpen(uv_work_t* req);
      void EIO_Open(GenericBaton* baton);
      static void EIO_AfterOpen(uv_work_t* req);
      
      static void StaticWrite(uv_work_t* req);
      void EIO_Write(QueuedWrite* baton);
      static void EIO_AfterWrite(uv_work_t* req);
      
      static void StaticQuery(uv_work_t* req);
      void EIO_Query(QueuedWrite* queuedWrite);
      static void EIO_AfterQuery(uv_work_t* req);
  }; 
  
  struct ListResultItem {
    public:
      std::string path;
      std::string idn;
  };
  
  struct GenericBaton {
    public:
      char command[QUERY_STRING_SIZE];
      NanCallback* callback;
      VisaEmitter* obj;
      char result[QUERY_STRING_SIZE];
      char errorString[ERROR_STRING_SIZE];
  };
  
  struct ListBaton {
    public:
      NanCallback* callback;
      std::list<ListResultItem*> results;
      char errorString[ERROR_STRING_SIZE];
      VisaEmitter* obj;
  };
  
  struct QueuedWrite {
    public:
      uv_work_t req;
      QUEUE queue;
      GenericBaton* baton;
      VisaEmitter* obj;
  };
}; /* namespace raw */


#endif 