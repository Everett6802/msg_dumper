#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include "msg_dumper.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Marcos
#define MSG_DUMPER_SHORT_STRING_SIZE			32U
#define MSG_DUMPER_STRING_SIZE 				64U
#define MSG_DUMPER_LONG_STRING_SIZE 			256U
#define MSG_DUMPER_EX_LONG_STRING_SIZE 		(MSG_DUMPER_LONG_STRING_SIZE * 2)

#ifndef CHECK_SUCCESS
#define CHECK_SUCCESS(x) (x == MSG_DUMPER_SUCCESS ? true : false)
#endif

#ifndef CHECK_FAILURE
#define CHECK_FAILURE(x) !CHECK_SUCCESS(x)
#endif

//#define SHOW_SYSLOG
#define LOG_BACKEND_NONE     0
#define LOG_BACKEND_SYSLOG   1
#define LOG_BACKEND_CONSOLE  2

#ifndef LOG_BACKEND
#define LOG_BACKEND LOG_BACKEND_NONE
#endif

#define NEED_SQL_DUMPER_NO   0
#define NEED_SQL_DUMPER_YES  1

#ifndef NEED_SQL_DUMPER
#define NEED_SQL_DUMPER NEED_SQL_DUMPER_NO
#endif

#define WRITE_SYSLOG_BEGIN()\
char title[64];\
snprintf(title, 64, "%s:%d", __FILE__, __LINE__);\
openlog(title, LOG_PID | LOG_CONS, LOG_USER);

#define WRITE_SYSLOG_END()\
closelog();

#define WRITE_SYSLOG(priority, fmt, ...)\
do{\
WRITE_SYSLOG_BEGIN()\
syslog(priority, "[MsgDumper %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
WRITE_SYSLOG_END()\
}while(0)

#define WRITE_CONSOLE(priority, fmt, ...)\
do{\
if (priority == LOG_ERR) fprintf(stderr, "[MsgDumper %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
else printf("[MsgDumper %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}while(0)

#if LOG_BACKEND == LOG_BACKEND_SYSLOG
#define WRITE_LOG_INTERNAL(priority, fmt, ...) WRITE_SYSLOG(priority, fmt, ##__VA_ARGS__)
#elif LOG_BACKEND == LOG_BACKEND_CONSOLE
#define WRITE_LOG_INTERNAL(priority, fmt, ...) WRITE_CONSOLE(priority, fmt, ##__VA_ARGS__)
#else
#define WRITE_LOG_INTERNAL(priority, fmt, ...) do {} while(0)
#endif

#define WRITE_DEBUG(fmt, ...) WRITE_LOG_INTERNAL(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define WRITE_INFO(fmt, ...) WRITE_LOG_INTERNAL(LOG_INFO, fmt, ##__VA_ARGS__)
#define WRITE_ERROR(fmt, ...) WRITE_LOG_INTERNAL(LOG_ERR, fmt, ##__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
extern const char* MODULE_NAME;
extern const unsigned char MAJOR_VERSION;
extern const unsigned char MINOR_VERSION;
extern const unsigned char BUILD_VERSION;
extern const char* MSG_DUMPER_ERROR_COLOR;
extern const char* MSG_DUMPER_SEVERITY_DESC[];
extern const int MSG_DUMPER_SEVERITY_DESC_LEN;
extern const char* MSG_DUMPER_SEVERITY_DESC_NOSET;
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
#if NEED_SQL_DUMPER == NEED_SQL_DUMPER_YES
enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_SQL, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_STDOUT, FACILITY_SIZE};
#else
enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_STDOUT, FACILITY_SIZE};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function
unsigned short transform_linux_severity_to_msg_dumper_severity(unsigned short linux_severity);
unsigned short transform_msg_dumper_severity_to_linux_severity(unsigned short msg_dumper_severity);
void convert_str2upper(char *str);
void convert_str2lower(char *str);
int get_facility_index_from_string(const char* facility);
int get_severity_index_from_string(const char* severity);
const char* get_facility_string_from_index(int facility_index);
const char* get_severity_string_from_index(int facility_index);
unsigned short safe_snprintf(char **out_buf, size_t init_size, const char *fmt, ...);

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
	unsigned short msg_dumper_severity;
	char data[MSG_DUMPER_LONG_STRING_SIZE];

	MsgCfg(const time_t& new_timep, unsigned short new_msg_dumper_severity, const char* new_data);
	const char* to_string();
};
typedef MsgCfg* PMSG_CFG;

////////////////////////////////////////////////////////////////////////////////////////////

class ScopedCStr
{
private:
    char *cstr;

// 禁止 copy constructor and assignment operator
// private → 外部不能呼叫; 沒有實作 → 就算內部誤用也會 link error
// 禁止 copy constructor
    ScopedCStr(const ScopedCStr&);
// 禁止 assignment operator
    ScopedCStr& operator=(const ScopedCStr&);

public:
    ScopedCStr();
    ~ScopedCStr();

// 允許 move copy constructor and move assignment operator
    ScopedCStr(ScopedCStr&& other);
    ScopedCStr& operator=(ScopedCStr&& other);

    char** out(){return &cstr;}   // 給 safe_snprintf 用
    const char* get()const{return cstr == NULL ? "" : cstr;}
	char* release();
};

#endif
