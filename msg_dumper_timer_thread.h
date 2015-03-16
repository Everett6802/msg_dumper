#ifndef MSG_DUMPER_TIMER_THREAD_H
#define MSG_DUMPER_TIMER_THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include "common.h"
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


class MsgDumperTimerThread : public MsgDumperBase
{
	friend class MsgDumperMgr;

protected:

private:
	pthread_t pid;
	int exit;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	bool thread_is_running;
	bool new_data_trigger;

	static void* msg_dumper_thread_handler(void* void_ptr);
	unsigned short msg_dumper_thread_handler_internal();

protected:
	vector<PMSG_CFG> buffer_vector;
	vector<PMSG_CFG> write_vector;
	char worker_thread_name[MSG_DUMPER_SHORT_STRING_SIZE];

	MsgDumperTimerThread();
	virtual ~MsgDumperTimerThread();

	pthread_t get_pid()const{return pid;}
	virtual const char* get_thread_name()const;

	virtual unsigned short create_device_file()=0;
	virtual unsigned short write_device_file()=0;

public:
	virtual unsigned short initialize(const char* conf_path, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(const time_t& timep, unsigned short severity, const char* msg);
};

#endif
