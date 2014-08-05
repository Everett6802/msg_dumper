#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"

using namespace std;


class MsgDumperLog : public MsgDumperBase
{
	friend class MsgDumperMgr;
private:
	static char* LOG_FOLDER ;

	pthread_t pid;
	int exit;
	char* log_filename;
	char* log_filepath;
	vector<char*> buffer_vector;
	vector<char*> write_vector;


	MsgDumperLog();

	static void* msg_dumper_log_handler(void* void_ptr);
	unsigned short msg_dumper_log_handler_internal();

	unsigned short create_logfile();
	unsigned short write_logfile();
	unsigned short create_logfolder();

public:
	virtual unsigned short initialize(void* config=NULL);
	virtual unsigned short write_msg(const char* msg);
	virtual unsigned short deinitialize();
};

#endif
