#include "common.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
const char* MODULE_NAME = "MSG_DUMPER";
const unsigned char MAJOR_VERSION = 1;
const unsigned char MINOR_VERSION = 0;
const char* MSG_DUMPER_SEVERITY_DESC[] = {"Error", "Warn", "Info", "Debug"};
const int MSG_DUMPER_SEVERITY_DESC_LEN = sizeof(MSG_DUMPER_SEVERITY_DESC) / sizeof(MSG_DUMPER_SEVERITY_DESC[0]);
const char* MSG_DUMPER_FACILITY_DESC[] = {"Log", "Com", "Sql", "Remote", "Syslog"};
const int MSG_DUMPER_FACILITY_DESC_LEN = sizeof(MSG_DUMPER_FACILITY_DESC) / sizeof(MSG_DUMPER_FACILITY_DESC[0]);

//const unsigned short MSG_DUMPER_SHORT_STRING_SIZE = DEF_MSG_DUMPER_SHORT_STRING_SIZE;
//const unsigned short MSG_DUMPER_STRING_SIZE = DEF_MSG_DUMPER_STRING_SIZE;
//const unsigned short MSG_DUMPER_LONG_STRING_SIZE = DEF_MSG_DUMPER_LONG_STRING_SIZE;
//const unsigned short MSG_DUMPER_EX_LONG_STRING_SIZE = DEF_MSG_DUMPER_LONG_STRING_SIZE * 2;

const unsigned short SERVER_PORT_NO = 6802;
const unsigned short MAX_CONNECTED_CLIENT = 5;

const char* error_description[] =
{
	"Success",
	"Failure Unknown",
	"Failure Invalid Argument",
	"Failure Invalid Pointer",
	"Failure Insufficient Memory",
	"Failure Open File",
	"Failure Not Found",
	"Failure Incorrect Config",
	"Failure Incorrect Operation",
	"Failure COM Port",
	"Failure MySQL",
	"Failure Socket"
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
MsgCfg::MsgCfg(const time_t& new_timep, unsigned short new_severity, const char* new_data)
{
	timep = new_timep;
	p = localtime(&timep);
	snprintf(date_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d/%02d/%02d", (1900 + p->tm_year) % 2000, p->tm_mon + 1, p->tm_mday);
	snprintf(time_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);

	severity = new_severity;
	memset(data, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
	snprintf(data, MSG_DUMPER_LONG_STRING_SIZE, "%s", new_data);

	format_message = NULL;
}

const char* MsgCfg::to_string()
{
	if (format_message == NULL)
	{
		format_message = new char[MSG_DUMPER_LONG_STRING_SIZE];
		if (format_message == NULL)
		{
			assert(0 && "Fail to allocate the format_message");
			return NULL;
		}
		snprintf(format_message, MSG_DUMPER_LONG_STRING_SIZE, "[%s %s %s] %s\n", date_str, time_str, MSG_DUMPER_SEVERITY_DESC[severity], data);
	}
	return format_message;
}
