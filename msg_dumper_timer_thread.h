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


class MsgDumperTimerThread
{
private:
	pthread_t pid;
	int exit;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	bool thread_is_running;
	bool new_data_trigger;

	MsgDumperBase* msg_dumper;

	static void* msg_dumper_thread_handler(void* void_ptr);
	unsigned short msg_dumper_thread_handler_internal();

protected:
	std::vector<PMSG_CFG> buffer_vector;
	std::vector<PMSG_CFG> write_vector;
	char worker_thread_name[MSG_DUMPER_SHORT_STRING_SIZE];

	pthread_t get_pid()const{return pid;}

public:
	MsgDumperTimerThread(MsgDumperBase* msg_dumper_obj);
	~MsgDumperTimerThread();

	unsigned short initialize(const char* current_working_directory, void* config=NULL);
	unsigned short deinitialize();
	unsigned short write_msg(const time_t& timep, unsigned short msg_dumper_severity, const char* msg);
};

#endif
