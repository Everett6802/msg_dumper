#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <stdio.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


class MsgDumperLog : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	static char* LOG_FOLDER;

	char* log_filename;
	char* log_filepath;

	unsigned short create_log_folder();

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperLog();
	virtual ~MsgDumperLog();

	virtual unsigned short initialize(void* config=NULL);
};

#endif
