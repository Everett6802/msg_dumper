#ifndef MSG_DUMPER_DAEMON_H
#define MSG_DUMPER_DAEMON_H

#include <syslog.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macro

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

#define WRITE_DEBUG_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_DEBUG, message_format, __VA_ARGS__)

#define WRITE_INFO_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_INFO, message_format, __VA_ARGS__)

#define WRITE_WARN_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_WARNING, message_format, __VA_ARGS__)

#define WRITE_ERR_FORMAT_SYSLOG(buf_size, message_format, ...)\
WRITE_FORMAT_SYSLOG(buf_size, LOG_ERR, message_format, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

#define WRITE_SYSLOG(priority, message)\
WRITE_SYSLOG_BEGIN()\
syslog(priority, message);\
WRITE_SYSLOG_END()

#define WRITE_DEBUG_SYSLOG(message)\
WRITE_SYSLOG(LOG_DEBUG, message)

#define WRITE_INFO_SYSLOG(message)\
WRITE_SYSLOG(LOG_INFO, message)

#define WRITE_WARN_SYSLOG(message)\
WRITE_SYSLOG(LOG_WARNING, message)

#define WRITE_ERR_SYSLOG(message)\
WRITE_SYSLOG(LOG_ERR, message)


#endif
