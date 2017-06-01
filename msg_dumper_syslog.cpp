#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "msg_dumper_syslog.h"


const char* MsgDumperSyslog::MSG_DUMPER_TITLE = "MsgDumper";
const char* MsgDumperSyslog::DEF_SYSLOG_FACILITY_NAME = "daemon";

MsgDumperSyslog::MsgDumperSyslog()
{
	memcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_SYSLOG], strlen(MSG_DUMPER_FACILITY_DESC[FACILITY_SYSLOG]));

	memset(syslog_facility_name, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(syslog_facility_name, DEF_SYSLOG_FACILITY_NAME, sizeof(char) * strlen(DEF_SYSLOG_FACILITY_NAME));
}

MsgDumperSyslog::~MsgDumperSyslog()
{
}

unsigned short MsgDumperSyslog::get_facility_number()
{
	static const char* syslog_facility_name_array[] = {
		"kern", "user", "mail", "daemon", "auth", "syslog", "lpr", "News", "Uucp", "cron", "authpriv", "ftp",
		"local0", "local1", "local2", "local3", "local4", "local5", "local6", "local7"
	};
	static int facility_array[] = {
		LOG_KERN, LOG_USER, LOG_MAIL, LOG_DAEMON, LOG_AUTH, LOG_SYSLOG, LOG_LPR, LOG_NEWS, LOG_UUCP, LOG_CRON, LOG_AUTHPRIV, LOG_FTP,
		LOG_LOCAL0, LOG_LOCAL1, LOG_LOCAL2, LOG_LOCAL3, LOG_LOCAL4, LOG_LOCAL5, LOG_LOCAL6, LOG_LOCAL7
	};
	static const int facility_len = sizeof facility_array / sizeof facility_array[0];

	for (int i = 0 ; i < facility_len ; i++)
	{
		if (strcmp(syslog_facility_name_array[i], syslog_facility_name) == 0)
		{
			facility_number = facility_array[i];
			return MSG_DUMPER_SUCCESS;
		}
	}

	WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter, fail to find the facility number: %s", syslog_facility_name);
	return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
}

unsigned short MsgDumperSyslog::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static const char* title[] = {"facility_name"};
	static int title_len = sizeof title / sizeof title[0];

	unsigned short ret = MSG_DUMPER_SUCCESS;
	bool found = false;
	for (int index = 0 ; index < title_len ; index++)
	{
		if (strcmp(title[index], param_title) == 0)
		{
			int param_content_len = strlen(param_content);
			char* param_member_variable = NULL;
			switch(index)
			{
			case 0:
				param_member_variable = syslog_facility_name;
				break;
			}

			if (param_member_variable != NULL)
			{
				memset(param_member_variable, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
				memcpy(param_member_variable, param_content, param_content_len);
				found = true;
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Update parameter: %s=%s", param_title, param_content);
			}
			else
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter: %s=%s", param_title, param_content);
				ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			}
			break;
		}
	}
// If the title is NOT found...
	if (!found)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter, fail to find the title: %s", param_title);
		ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	return ret;
}

unsigned short MsgDumperSyslog::open_device()
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::close_device()
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::initialize(const char* current_working_directory, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperSyslog object......");

// Parse the config file first
	unsigned short ret = parse_config(current_working_directory, "syslog");
	if (CHECK_FAILURE(ret))
		return ret;

// Get the facility number
	ret = get_facility_number();
	if (CHECK_FAILURE(ret))
		return ret;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::deinitialize()
{
	WRITE_DEBUG_SYSLOG("DeInitialize the MsgDumperSyslog object......");

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSyslog::write_msg(PMSG_CFG msg_cfg)
{
	static int SyslogLevel[] = {LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG};

// Connect to syslog server
	snprintf(title, 64, "[%s:%s]", MSG_DUMPER_TITLE, MSG_DUMPER_SEVERITY_DESC[msg_cfg->severity]);
	openlog(title, /*LOG_PID |*/ LOG_CONS, facility_number);

//	snprintf(syslog_buf, MSG_DUMPER_EX_LONG_STRING_SIZE, "%s%s", MSG_DUMPER_TITLE, msg);
	syslog(SyslogLevel[msg_cfg->severity], "%s", msg_cfg->data);

// Close the syslog
	closelog();

	return MSG_DUMPER_SUCCESS;
}
