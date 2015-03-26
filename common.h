#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
static char* MODULE_NAME = "MSG_DUMPER";
static unsigned char MAJOR_VERSION = 1;
static unsigned char MINOR_VERSION = 0;
static char* MSG_DUMPER_SEVERITY_DESC[] = {"Error", "Warn", "Info", "Debug"};
static char* MSG_DUMPER_FACILITY_DESC[] = {"Log", "Com", "Sql", "Remote", "Syslog"};

static const unsigned short MSG_DUMPER_SHORT_STRING_SIZE = 32;
static const unsigned short MSG_DUMPER_STRING_SIZE = 64;
static const unsigned short MSG_DUMPER_LONG_STRING_SIZE = 256;
static const unsigned short MSG_DUMPER_EX_LONG_STRING_SIZE = MSG_DUMPER_LONG_STRING_SIZE * 2;

static const unsigned short SERVER_PORT_NO = 6802;
static const unsigned short MAX_CONNECTED_CLIENT = 5;

static const char* error_description[] =
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
// Enumeration
enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_SQL, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_SIZE};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macro

#define SHOW_SYSLOG

#define WRITE_SYSLOG_BEGIN()\
do{\
char title[64];\
snprintf(title, 64, "%s:%d", __FILE__, __LINE__);\
openlog(title, LOG_PID | LOG_CONS, LOG_USER);

#define WRITE_SYSLOG_END()\
closelog();\
}while(0)

#define WRITE_FORMAT_SYSLOG(syslog_buf_size, priority, message_format, ...)\
WRITE_SYSLOG_BEGIN()\
char syslog_buf[syslog_buf_size];\
snprintf(syslog_buf, syslog_buf_size, message_format, __VA_ARGS__);\
syslog(priority, syslog_buf);\
WRITE_SYSLOG_END()

#if defined SHOW_SYSLOG
#define WRITE_DEBUG_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_DEBUG, message_format, __VA_ARGS__)
#else
#define WRITE_DEBUG_FORMAT_SYSLOG(buf_size, message_format, ...)
#endif

#if defined SHOW_SYSLOG
#define WRITE_INFO_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_INFO, message_format, __VA_ARGS__)
#else
#define WRITE_INFO_FORMAT_SYSLOG(buf_size, message_format, ...)
#endif

#if defined SHOW_SYSLOG
#define WRITE_ERR_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_ERR, message_format, __VA_ARGS__)
#else
#define WRITE_ERR_FORMAT_SYSLOG(buf_size, message_format, ...)
#endif

#define WRITE_SYSLOG(priority, message)\
WRITE_SYSLOG_BEGIN()\
syslog(priority, message);\
WRITE_SYSLOG_END()

#if defined SHOW_SYSLOG
#define WRITE_DEBUG_SYSLOG(message)\
WRITE_SYSLOG(LOG_DEBUG, message)
#else
#define WRITE_DEBUG_SYSLOG(message)
#endif

#if defined SHOW_SYSLOG
#define WRITE_INFO_SYSLOG(message)\
WRITE_SYSLOG(LOG_INFO, message)
#else
#define WRITE_INFO_SYSLOG(message)
#endif

#if defined SHOW_SYSLOG
#define WRITE_ERR_SYSLOG(message)\
WRITE_SYSLOG(LOG_ERR, message)
#else
#define WRITE_ERR_SYSLOG(message)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class
class MsgCfg
{
private:
	time_t timep;
	struct tm *p;
	char* format_message;

public:
	char date_str[MSG_DUMPER_SHORT_STRING_SIZE];
	char time_str[MSG_DUMPER_SHORT_STRING_SIZE];
	unsigned short severity;
	char data[MSG_DUMPER_LONG_STRING_SIZE];

	MsgCfg(const time_t& new_timep, unsigned short new_severity, const char* new_data);
	const char* to_string();
};
typedef MsgCfg* PMSG_CFG;


#endif
