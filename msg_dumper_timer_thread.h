#ifndef MSG_DUMPER_TIMER_THREAD_H
#define MSG_DUMPER_TIMER_THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


using namespace std;

class MsgDumperTimerThread : public MsgDumperBase
{
	friend class MsgDumperMgr;
private:
	pthread_t pid;
	int exit;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	bool thread_is_running;

	static void* msg_dumper_thread_handler(void* void_ptr);
	unsigned short msg_dumper_thread_handler_internal();

protected:
	static const int CURRENT_TIME_STRING_LENGTH;

	vector<char*> buffer_vector;
	vector<char*> write_vector;

	MsgDumperTimerThread();
	virtual ~MsgDumperTimerThread();

	unsigned short generate_current_time_string(char* current_time_string);

	virtual unsigned short create_device_file()=0;
	virtual unsigned short write_device_file()=0;

public:
	virtual unsigned short initialize(void* config=NULL);
	virtual unsigned short write_msg(const char* msg);
	virtual unsigned short deinitialize();
};

#endif
