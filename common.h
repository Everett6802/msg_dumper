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
// Marcos
#define MSG_DUMPER_SHORT_STRING_SIZE			32U
#define MSG_DUMPER_STRING_SIZE 					64U
#define MSG_DUMPER_LONG_STRING_SIZE 			256U
#define MSG_DUMPER_EX_LONG_STRING_SIZE 			(MSG_DUMPER_LONG_STRING_SIZE * 2)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
extern const char* MODULE_NAME;
extern const unsigned char MAJOR_VERSION;
extern const unsigned char MINOR_VERSION;
extern const char* MSG_DUMPER_SEVERITY_DESC[];
extern const int MSG_DUMPER_SEVERITY_DESC_LEN;
extern const char* MSG_DUMPER_FACILITY_DESC[];
extern const int MSG_DUMPER_FACILITY_DESC_LEN;

//extern const unsigned short MSG_DUMPER_SHORT_STRING_SIZE;
//extern const unsigned short MSG_DUMPER_STRING_SIZE;
//extern const unsigned short MSG_DUMPER_LONG_STRING_SIZE;
//extern const unsigned short MSG_DUMPER_EX_LONG_STRING_SIZE;

extern const unsigned short SERVER_PORT_NO;
extern const unsigned short MAX_CONNECTED_CLIENT;

extern const char* error_description[];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration
enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_SQL, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_SIZE};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macro

//#define SHOW_SYSLOG

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
