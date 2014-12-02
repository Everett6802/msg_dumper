#ifndef MSG_DUMPER_REMOTE_H
#define MSG_DUMPER_REMOTE_H

#include <stdio.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


class MsgDumperRemote : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();

public:
	MsgDumperRemote();
	virtual ~MsgDumperRemote();

	virtual unsigned short initialize(void* config=NULL);
};

#endif
