#ifndef MSG_DUMPER_COM_H
#define MSG_DUMPER_COM_H

#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


class MsgDumperCom : public MsgDumperBase
{
	friend class MsgDumperMgr;
private:
	MsgDumperCom();

public:
	virtual unsigned short initialize(void* config);
	virtual unsigned short write_msg(const char* msg);
	virtual unsigned short deinitialize();
};

#endif
