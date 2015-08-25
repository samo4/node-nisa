#include "nisa.h"

namespace raw {
	void VisaEmitter::StaticOpen(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_Open(data);
	} 
	
	void VisaEmitter::StaticWrite(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_Write(data);
	}
	
	void VisaEmitter::StaticDeviceClear(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_DeviceClear(data);
	}
	
	void VisaEmitter::StaticRead(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_Read(data);
	}
	
	void VisaEmitter::StaticQuery(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
		obj->EIO_Query(data);
	}
	
	void VisaEmitter::StaticTrigger(uv_work_t* req) {
		QueuedWrite* data = static_cast<QueuedWrite*>(req->data);	
		VisaEmitter* obj = static_cast<VisaEmitter*>(data->obj);
			obj->EIO_Trigger(data);
	}
}