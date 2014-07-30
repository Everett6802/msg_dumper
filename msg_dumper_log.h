#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <pthread.h>
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


class MsgDumperLog : public MsgDumperBase
{
	friend class MsgDumperMgr;
private:
	pthread_t pid;
	int exit;

	MsgDumperLog();

	static void* msg_dumper_log_handler(void* void_ptr);
	unsigned short msg_dumper_log_handler_internal();

public:
	virtual unsigned short initialize(void* config);
	virtual unsigned short write_msg(const char* msg);
	virtual unsigned short deinitialize();
};

#endif
