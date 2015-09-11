#ifndef MSG_DUMPER_SYSLOG_H
#define MSG_DUMPER_SYSLOG_H

#include <stdio.h>
#include <syslog.h>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperSyslog : public MsgDumperBase
{
private:
	static const char* MSG_DUMPER_TITLE;
	static const char* DEF_SYSLOG_FACILITY_NAME;

	char title[64];
	char syslog_buf[MSG_DUMPER_EX_LONG_STRING_SIZE];
	char syslog_facility_name[MSG_DUMPER_STRING_SIZE];
	int facility_number;

	unsigned short get_facility_number();

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperSyslog();
	virtual ~MsgDumperSyslog();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* config_path, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
