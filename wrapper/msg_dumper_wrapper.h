#ifndef MSG_DUMPER_WRAPPER_H
#define MSG_DUMPER_WRAPPER_H

#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "msg_dumper.h"


#define MSG_DUMPER_BUF_SIZE 512

#define DECLARE_MSG_DUMPER()\
MsgDumperWrapper* msg_dumper;

#define DECLARE_MSG_DUMPER_PARAM()\
static int __msg_dumper_title_len__;\
static int __msg_dumper_message_len__;\
static char __msg_dumper_message__[MSG_DUMPER_BUF_SIZE];

#define IMPLEMENT_MSG_DUMPER()\
msg_dumper = MsgDumperWrapper::get_instance();\

// Can be used for functions
#define DECLARE_AND_IMPLEMENT_STATIC_MSG_DUMPER()\
static MsgDumperWrapper* msg_dumper = MsgDumperWrapper::get_instance();\

#define RELEASE_MSG_DUMPER()\
if (msg_dumper != NULL)\
{\
	msg_dumper->release();\
	msg_dumper = NULL;\
}

#define SHOW_MSG_DUMPER

#define WRITE_MSG_DUMPER_BEGIN()\
do{\
snprintf(__msg_dumper_message__, MSG_DUMPER_BUF_SIZE, "(%s:%d) ", __FILE__, __LINE__);\
__msg_dumper_title_len__ = strlen(__msg_dumper_message__);\
__msg_dumper_message_len__ = MSG_DUMPER_BUF_SIZE - __msg_dumper_title_len__;

#define WRITE_MSG_DUMPER_END()\
}while(0)

#define SET_SEVERITY_LEVEL(func_name, severity_level)\
msg_dumper->func_name(severity_level);

#define GET_SEVERITY_LEVEL(func_name)\
msg_dumper->func_name();

#define STATIC_SET_SEVERITY_LEVEL(func_name, severity_level)\
do{\
DECLARE_AND_IMPLEMENT_STATIC_MSG_DUMPER();\
SET_SEVERITY_LEVEL(func_name, severity_level);\
RELEASE_MSG_DUMPER();\
}while(0);

#define WRITE_MSG_DUMPER(priority, message)\
WRITE_MSG_DUMPER_BEGIN()\
snprintf(&__msg_dumper_message__[__msg_dumper_title_len__], __msg_dumper_message_len__, "%s", message);\
msg_dumper->write(priority, __msg_dumper_message__);\
WRITE_MSG_DUMPER_END()

#define WRITE_FORMAT_MSG_DUMPER(priority, message_format, ...)\
WRITE_MSG_DUMPER_BEGIN()\
snprintf(&__msg_dumper_message__[__msg_dumper_title_len__], __msg_dumper_message_len__, message_format, __VA_ARGS__);\
msg_dumper->write(priority, __msg_dumper_message__);\
WRITE_MSG_DUMPER_END()

#define STATIC_WRITE_MSG_DUMPER(priority, message)\
do{\
DECLARE_AND_IMPLEMENT_STATIC_MSG_DUMPER();\
DECLARE_MSG_DUMPER_PARAM();\
WRITE_MSG_DUMPER(priority, message);\
RELEASE_MSG_DUMPER();\
}while(0);

#define STATIC_WRITE_FORMAT_MSG_DUMPER(priority, message_format, ...)\
do{\
DECLARE_AND_IMPLEMENT_STATIC_MSG_DUMPER();\
DECLARE_MSG_DUMPER_PARAM();\
WRITE_FORMAT_MSG_DUMPER(priority, message_format, __VA_ARGS__);\
RELEASE_MSG_DUMPER();\
}while(0);

#if defined SHOW_MSG_DUMPER

#define SET_LOG_SEVERITY(severity_level) SET_SEVERITY_LEVEL(set_log_severity, severity_level)
#define SET_SYSLOG_SEVERITY(severity_level) SET_SEVERITY_LEVEL(set_syslog_severity, severity_level)
#define GET_LOG_SEVERITY() GET_SEVERITY_LEVEL(get_log_severity)
#define GET_SYSLOG_SEVERITY() GET_SEVERITY_LEVEL(get_syslog_severity)

#define SET_LOG_SEVERITY_CONFIG(severity_level) SET_SEVERITY_LEVEL(set_log_severity_config, severity_level)
#define SET_SYSLOG_SEVERITY_CONFIG(severity_level) SET_SEVERITY_LEVEL(set_syslog_severity_config, severity_level)
#define GET_LOG_SEVERITY_CONFIG() GET_SEVERITY_LEVEL(get_log_severity_config)
#define GET_SYSLOG_SEVERITY_CONFIG() GET_SEVERITY_LEVEL(get_syslog_severity_config)

#define STATIC_SET_LOG_SEVERITY(severity_level) STATIC_SET_SEVERITY_LEVEL(set_log_severity, severity_level)
#define STATIC_SET_SYSLOG_SEVERITY(severity_level) STATIC_SET_SEVERITY_LEVEL(set_syslog_severity, severity_level)

#define STATIC_SET_LOG_SEVERITY_CONFIG(severity_level) STATIC_SET_SEVERITY_LEVEL(set_log_severity_config, severity_level)
#define STATIC_SET_SYSLOG_SEVERITY_CONFIG(severity_level) STATIC_SET_SEVERITY_LEVEL(set_syslog_severity_config, severity_level)

#define WRITE_DEBUG(message) WRITE_MSG_DUMPER(LOG_DEBUG, message)
#define WRITE_INFO(message) WRITE_MSG_DUMPER(LOG_INFO, message)
#define WRITE_WARN(message) WRITE_MSG_DUMPER(LOG_WARNING, message)
#define WRITE_ERROR(message) WRITE_MSG_DUMPER(LOG_ERR, message)

#define WRITE_FORMAT_DEBUG(message_format, ...) WRITE_FORMAT_MSG_DUMPER(LOG_DEBUG, message_format, __VA_ARGS__)
#define WRITE_FORMAT_INFO(message_format, ...) WRITE_FORMAT_MSG_DUMPER(LOG_INFO, message_format, __VA_ARGS__)
#define WRITE_FORMAT_WARN(message_format, ...) WRITE_FORMAT_MSG_DUMPER(LOG_WARNING, message_format, __VA_ARGS__)
#define WRITE_FORMAT_ERROR(message_format, ...) WRITE_FORMAT_MSG_DUMPER(LOG_ERR, message_format, __VA_ARGS__)

#define STATIC_WRITE_DEBUG(message) STATIC_WRITE_MSG_DUMPER(LOG_DEBUG, message)
#define STATIC_WRITE_INFO(message) STATIC_WRITE_MSG_DUMPER(LOG_INFO, message)
#define STATIC_WRITE_WARN(message) STATIC_WRITE_MSG_DUMPER(LOG_WARNING, message)
#define STATIC_WRITE_ERROR(message) STATIC_WRITE_MSG_DUMPER(LOG_ERR, message)

#define STATIC_WRITE_FORMAT_DEBUG(message_format, ...) STATIC_WRITE_FORMAT_MSG_DUMPER(LOG_DEBUG, message_format, __VA_ARGS__)
#define STATIC_WRITE_FORMAT_INFO(message_format, ...) STATIC_WRITE_FORMAT_MSG_DUMPER(LOG_INFO, message_format, __VA_ARGS__)
#define STATIC_WRITE_FORMAT_WARN(message_format, ...) STATIC_WRITE_FORMAT_MSG_DUMPER(LOG_WARNING, message_format, __VA_ARGS__)
#define STATIC_WRITE_FORMAT_ERROR(message_format, ...) STATIC_WRITE_FORMAT_MSG_DUMPER(LOG_ERR, message_format, __VA_ARGS__)

#else

#define WRITE_DEBUG(message)
#define WRITE_INFO(message)
#define WRITE_WARN(message)
#define WRITE_ERROR(message)

#define WRITE_FORMAT_DEBUG(message_format, ...)
#define WRITE_FORMAT_INFO(message_format, ...)
#define WRITE_FORMAT_WARN(message_format, ...)
#define WRITE_FORMAT_ERROR(message_format, ...)

#define STATIC_WRITE_DEBUG(message)
#define STATIC_WRITE_INFO(message)
#define STATIC_WRITE_WARN(message)
#define STATIC_WRITE_ERROR(message)

#define STATIC_WRITE_FORMAT_DEBUG(message_format, ...)
#define STATIC_WRITE_FORMAT_INFO(message_format, ...)
#define STATIC_WRITE_FORMAT_WARN(message_format, ...)
#define STATIC_WRITE_FORMAT_ERROR(message_format, ...)

#endif


class MsgDumperWrapper
{
private:
	static MsgDumperWrapper* instance;
	static const char* MSG_DUMPER_ERROR_COLOR;
	static const char* FACILITY_NAME[];
	static const int FACILITY_NAME_SIZE;
	static const unsigned short FACILITY_FLAG[];
	static const char* SEVERITY_NAME[];
	static const int SEVERITY_NAME_SIZE;
	static const char* CONF_FOLDER;
	static const char* CONF_FILENAME;

	int ref_count;
	void* api_handle;
	FP_msg_dumper_initialize fp_msg_dumper_initialize;
	FP_msg_dumper_get_version fp_msg_dumper_get_version;
	FP_msg_dumper_set_severity fp_msg_dumper_set_severity;
	FP_msg_dumper_set_facility fp_msg_dumper_set_facility;
	FP_msg_dumper_get_severity fp_msg_dumper_get_severity;
	FP_msg_dumper_get_facility fp_msg_dumper_get_facility;
	FP_msg_dumper_write_msg fp_msg_dumper_write_msg;
	FP_msg_dumper_deinitialize fp_msg_dumper_deinitialize;
	FP_msg_dumper_get_error_description fp_msg_dumper_get_error_description;

	unsigned short facility_flag;
	char *fmt_msg_buf;
	int fmt_msg_buf_size;

	MsgDumperWrapper();
	MsgDumperWrapper(const MsgDumperWrapper&);
	MsgDumperWrapper& operator=(const MsgDumperWrapper&);

	unsigned short initialize();
	void deinitialize();
	bool export_api();
	int get_severity_index(const char* severity)const;
	int get_facility_index(const char* facility)const;
	unsigned short parse_config();
	unsigned short set_severity(unsigned short severity_index, unsigned short facility_flag);
	unsigned short get_severity(unsigned short facility_flag)const;
	unsigned short set_config(const char* config_name, const char* config_value);
	unsigned short get_config(const char* config_name, char* config_value)const;

public:
	static const int STRING_BUF_SIZE;
	~MsgDumperWrapper(){deinitialize();}

	static MsgDumperWrapper* get_instance();
	int addref();
	int release();

	unsigned short set_log_severity(unsigned short log_severity);
	unsigned short set_syslog_severity(unsigned short syslog_severity);
	unsigned short get_log_severity()const;
	unsigned short get_syslog_severity()const;
	unsigned short set_log_severity_config(unsigned short log_severity);
	unsigned short set_syslog_severity_config(unsigned short syslog_severity);
	unsigned short get_log_severity_config()const;
	unsigned short get_syslog_severity_config()const;

	unsigned short write(unsigned short syslog_priority, const char* msg);
	unsigned short format_write_va(unsigned short syslog_priority, const char* msg_fmt, va_list ap);
	unsigned short format_write(unsigned short syslog_priority, const char* msg_fmt, ...);

	unsigned short error(const char* msg);
	unsigned short format_error(const char* msg_fmt, ...);
	unsigned short warn(const char* msg);
	unsigned short format_warn(const char* msg_fmt, ...);
	unsigned short info(const char* msg);
	unsigned short format_info(const char* msg_fmt, ...);
	unsigned short debug(const char* msg);
	unsigned short format_debug(const char* msg_fmt, ...);


	const char* get_error_description()const;
};

#endif

