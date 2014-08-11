#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"

using namespace std;


class MsgDumperLog : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	static char* LOG_FOLDER ;

	char* log_filename;
	char* log_filepath;

	MsgDumperLog();
	~MsgDumperLog();

	unsigned short create_log_folder();

protected:
	unsigned short create_device_file();
	unsigned short write_device_file();

public:
	virtual unsigned short initialize(void* config=NULL);
};

#endif
