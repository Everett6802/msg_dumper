#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <stdio.h>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperLog : public MsgDumperBase
{
private:
	static char* DEF_LOG_FOLDER;
	char log_folder[MSG_DUMPER_STRING_SIZE];

	char* log_filename;
	char* log_filepath;
	FILE* fp_log;

	unsigned short create_log_folder(const char* config_path);

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperLog();
	virtual ~MsgDumperLog();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* config_path, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
