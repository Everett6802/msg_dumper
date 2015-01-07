#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"


static MsgDumperMgr msg_dumepr_mgr;
static char fmtmsg[MSG_DUMPER_EX_LONG_STRING_SIZE];

unsigned short msg_dumper_get_version(unsigned char& major_version, unsigned char& minor_version)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	major_version = MAJOR_VERSION;
	minor_version = MINOR_VERSION;

	return MSG_DUMPER_SUCCESS;
}

unsigned short msg_dumper_initialize(const char* config_path)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.initialize(config_path);
}

unsigned short msg_dumper_set_severity(unsigned short severity)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.set_severity(severity);
}

unsigned short msg_dumper_set_facility(unsigned short facility)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.set_facility(facility);
}

unsigned short msg_dumper_get_severity()
{

}

unsigned short msg_dumper_get_severity_ex(unsigned short facility)
{

}

unsigned short msg_dumper_get_facility()
{

}

unsigned short msg_dumper_write_msg(unsigned short severity, const char* msg)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.write_msg(severity, msg);
}

unsigned short msg_dumper_write_format_msg(unsigned short severity, const char* fmt, ...)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);

	static const int fmtmsg_len = sizeof(fmtmsg) / sizeof(fmtmsg[0]);
	static const int fmtbuf_len = 16;
	static char fmtbuf[fmtbuf_len];

	if (severity > dumper_severity)
		return MSG_DUMPER_SUCCESS;

	if (fmt == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid pointer: format");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	va_list arg_list;
	va_start(arg_list, fmt);

// Parse the string format, and generate the string
	char* arg_str = NULL;
	int arg_str_len = 0;
	int fmtmsg_index = 0;
	bool out = false;
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
				}
				break;
			}
			arg_str_len = strlen(arg_str);
			if (fmtmsg_index + arg_str_len >= fmtmsg_len - 1)
			{
				arg_str_len = fmtmsg_len - fmtmsg_index - 1;
				out = true;
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

	return msg_dumper_write_msg(severity, fmtmsg);
}

unsigned short msg_dumper_deinitialize()
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.deinitialize();
}

