#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_syslog.h"


char* MsgDumperSyslog::msg_dumper_title = "MsgDumper";

MsgDumperSyslog::MsgDumperSyslog()
{
}

MsgDumperSyslog::~MsgDumperSyslog()
{
}

unsigned short MsgDumperSyslog::initialize(const char* config_path, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperSyslog object......");
	snprintf(title, 64, "%s ", msg_dumper_title);
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::deinitialize()
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::write_msg(const time_t& timep, unsigned short severity, const char* msg)
{
	static int SyslogLevel[] = {LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG};

	snprintf(title, 64, "[%s:%s]", msg_dumper_title, MSG_DUMPER_SEVERITY_DESC[severity]);
	openlog(title, /*LOG_PID |*/ LOG_CONS, LOG_DAEMON);
//	snprintf(syslog_buf, MSG_DUMPER_EX_LONG_STRING_SIZE, "%s%s", msg_dumper_title, msg);
	syslog(SyslogLevel[severity], msg);

	return MSG_DUMPER_SUCCESS;
}
