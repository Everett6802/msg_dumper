#ifndef MSG_DUMPER_STDOUT_H
#define MSG_DUMPER_STDOUT_H

#include <stdio.h>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperStdout : public MsgDumperBase
{
private:
	static const char* MSG_DUMPER_TITLE;

	char title[64];
	int normal_output_type;
	int error_output_type;

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperStdout();
	virtual ~MsgDumperStdout();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* current_working_directory, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
