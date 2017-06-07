#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "msg_dumper.h"
#include "msg_dumper_mgr.h"



static MsgDumperMgr msg_dumper_mgr;
static unsigned short last_error = MSG_DUMPER_SUCCESS;

unsigned short msg_dumper_get_version(unsigned char& major_version, unsigned char& minor_version, unsigned char& build_version)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	major_version = MAJOR_VERSION;
	minor_version = MINOR_VERSION;
	build_version = BUILD_VERSION;

	return MSG_DUMPER_SUCCESS;
}

unsigned short msg_dumper_initialize()
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	last_error = msg_dumper_mgr.initialize();
	return last_error;
}

unsigned short msg_dumper_set_severity(unsigned short linux_severity, unsigned short facility)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	unsigned short msg_dumper_severity = transform_linux_severity_to_msg_dumper_severity(linux_severity);
	last_error = msg_dumper_mgr.set_severity(msg_dumper_severity, facility);
	return last_error;
}

unsigned short msg_dumper_set_severity_all(unsigned short linux_severity)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	unsigned short msg_dumper_severity = transform_linux_severity_to_msg_dumper_severity(linux_severity);
	last_error = msg_dumper_mgr.set_severity_all(msg_dumper_severity);
	return last_error;
}

unsigned short msg_dumper_set_facility(unsigned short facility)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	last_error = msg_dumper_mgr.set_facility(facility);
	return last_error;
}

unsigned short msg_dumper_get_severity(unsigned short facility)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	unsigned short msg_dumper_severity = msg_dumper_mgr.get_severity(facility);
	unsigned short linux_severity = transform_msg_dumper_severity_to_linux_severity(msg_dumper_severity);
	last_error = MSG_DUMPER_SUCCESS;
	return linux_severity;
}

unsigned short msg_dumper_get_facility()
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	unsigned short facility = msg_dumper_mgr.get_facility();
	last_error = MSG_DUMPER_SUCCESS;
	return facility;
}

unsigned short msg_dumper_write_msg(unsigned short linux_severity, const char* msg)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	// fprintf(stderr, "msg_dumper_write_msg linux_severity: %d\n", linux_severity);
	unsigned short msg_dumper_severity = transform_linux_severity_to_msg_dumper_severity(linux_severity);
	// fprintf(stderr, "msg_dumper_write_msg msg_dumper_severity: %d\n", msg_dumper_severity);
	if (msg_dumper_mgr.can_ignore(msg_dumper_severity))
		last_error = MSG_DUMPER_SUCCESS;
	else
		last_error = msg_dumper_mgr.write_msg(msg_dumper_severity, msg);
	return last_error;
}

unsigned short msg_dumper_write_format_msg(unsigned short linux_severity, const char* fmt, ...)
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	static char fmtmsg[MSG_DUMPER_EX_LONG_STRING_SIZE];
	static const int fmtmsg_len = sizeof(fmtmsg) / sizeof(fmtmsg[0]);
	static const int fmtbuf_len = 16;
	static char fmtbuf[fmtbuf_len];

	if (fmt == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid pointer: format");
		last_error = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
		return last_error;
	}

	unsigned short msg_dumper_severity = transform_linux_severity_to_msg_dumper_severity(linux_severity);
	if (msg_dumper_mgr.can_ignore(msg_dumper_severity))
		last_error = MSG_DUMPER_SUCCESS;
	else
	{
		va_list arg_list;
		va_start(arg_list, fmt);

	// Parse the string format, and generate the string
		char* arg_str = NULL;
		int arg_str_len = 0;
		int fmtmsg_index = 0;
//		bool out = false;
		for(const char* p = fmt; *p != '\0' && fmtmsg_index < fmtmsg_len ; p++)
		{
			if (*p == '%')
			{
				switch (*++p)
				{
					case 'd':
					{
	// itoa is not ansi C standard and you should probably avoid it. Here are some roll-your-own implementations if you really want to use it anyway:
	//				fputs(itoa(value, fmtmsg, 10), stdout);
						snprintf(fmtbuf, fmtbuf_len, "%d", va_arg(arg_list, int));
						arg_str = fmtbuf;
					}
					break;
					case 's':
					{
						arg_str = va_arg(arg_list, char*);
					}
					break;
					default:
					{
						assert(0 && "Unsupported format");
						return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
					}
					break;
				}
				arg_str_len = strlen(arg_str);
				if (fmtmsg_index + arg_str_len >= fmtmsg_len - 1)
				{
					arg_str_len = fmtmsg_len - fmtmsg_index - 1;
//					out = true;
				}
				memcpy(&fmtmsg[fmtmsg_index], arg_str, sizeof(char) * arg_str_len);
				fmtmsg_index += arg_str_len;
		//		fputs(arg_str, stdout);
			}
			else
			{
	//			putchar(*p);
				fmtmsg[fmtmsg_index++] = *p;
			}
		}
		fmtmsg[fmtmsg_index] = '\0';
		va_end(arg_list);

		last_error = msg_dumper_mgr.write_msg(msg_dumper_severity, fmtmsg);
	}

	return last_error;
}

unsigned short msg_dumper_deinitialize()
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	last_error = msg_dumper_mgr.deinitialize();
	return last_error;
}

const char* msg_dumper_get_error_description()
{
	return error_description[last_error];
}
