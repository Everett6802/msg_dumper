#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"
#include "msg_dumper_sql.h"
#include "msg_dumper_remote.h"
#include "msg_dumper_syslog.h"


char* MsgDumperMgr::dev_name[] = {"Log", "Com", "Sql", "Remote", "Syslog"};
short MsgDumperMgr::dev_flag[] = {MSG_DUMPER_FACILITY_LOG, MSG_DUMPER_FACILITY_COM, MSG_DUMPER_FACILITY_SQL, MSG_DUMPER_FACILITY_REMOTE, MSG_DUMPER_FACILITY_SYSLOG};

MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_severity(MSG_DUMPER_SEVIRITY_ERROR),
	dumper_facility(MSG_DUMPER_FACILITY_LOG)
{
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		msg_dumper[i] = NULL;
}

unsigned short MsgDumperMgr::initialize(const char* config_path)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The config path: %s", config_path);

// Register the class to the simple factory
	REGISTER_CLASS(MsgDumperLog);
	REGISTER_CLASS(MsgDumperCom);
	REGISTER_CLASS(MsgDumperSql);
	REGISTER_CLASS(MsgDumperRemote);
	REGISTER_CLASS(MsgDumperSyslog);

	unsigned short ret = MSG_DUMPER_SUCCESS;
	char dev_class_name[32];
// Initialize the device
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		snprintf(dev_class_name, 32, "MsgDumper%s", dev_name[i]);
// Allocate and initialize the object derived from the MsgDumplerBase class
		if (dumper_facility & dev_flag[i])
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Allocate the %s object", dev_class_name);
			msg_dumper[i] = device_factory.construct(dev_class_name);
			if (msg_dumper[i] == NULL)
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to allocate the %s object", dev_class_name);
				return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
			}

			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Initialize the %s object", dev_class_name);
			ret = msg_dumper[i]->initialize(config_path);
			if (CHECK_MSG_DUMPER_FAILURE(ret))
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to initialize the %s object", dev_class_name);
				goto EXIT;
			}
		}
	}

	is_init = true;

EXIT:
	if (!is_init)
	{
		for (int i = 0 ; i < FACILITY_SIZE ; i++)
		{
			if (msg_dumper[i] != NULL)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Deinitialize the MsgDumper%s object due to the failure of initialization of some objects", dev_name[i]);
				msg_dumper[i]->deinitialize();
				delete msg_dumper[i];
				msg_dumper[i] = NULL;
			}
		}
	}
	return ret;
}

unsigned short MsgDumperMgr::set_severity(unsigned short severity)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity to %d", severity);
	dumper_severity = severity;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::set_facility(unsigned short facility)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the facility to %d", facility);
	dumper_facility = facility;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::write_msg(unsigned short severity, const char* msg)
{
	if (!is_init)
	{
		WRITE_ERR_SYSLOG("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	if (msg == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid pointer: msg");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
// Mark the time that the message is generated
	time_t timep;
	time(&timep);

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper[i])
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [%s] to %s", msg, dev_name[i]);
			ret = msg_dumper[i]->write_msg(timep, severity, msg);
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				return ret;
		}
	}

	return ret;
}

//unsigned short MsgDumperMgr::write_format_msg(unsigned short severity, const char* fmt, ...)
//{
//	static const int fmtmsg_len = sizeof(fmtmsg) / sizeof(fmtmsg[0]);
//	static const int fmtbuf_len = 16;
//	static char fmtbuf[fmtbuf_len];
//
//	if (!is_init)
//	{
//		WRITE_ERR_SYSLOG("Library is Not initialized");
//		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
//	}
//
//	if (severity > dumper_severity)
//		return MSG_DUMPER_SUCCESS;
//
//	if (format == NULL)
//	{
//		WRITE_ERR_SYSLOG("Invalid pointer: format");
//		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
//	}
//
//	va_list arg_list;
//	va_start(arg_list, fmt);
//
//// Parse the string format, and generate the string
//	char* arg_str = NULL;
//	int arg_str_len = 0;
//	int fmtmsg_index = 0;
//	bool out = false;
//	for(const char* p = fmt; *p != '\0' && fmtmsg_index < fmtmsg_len ; p++)
//	{
//		if (*p == '%')
//		{
//			switch (*++p)
//			{
//				case 'd':
//				{
//// itoa is not ansi C standard and you should probably avoid it. Here are some roll-your-own implementations if you really want to use it anyway:
////				fputs(itoa(value, fmtmsg, 10), stdout);
//					snprintf(fmtbuf, fmtbuf_len, "%d", va_arg(arg_list, int));
//					arg_str = fmtbuf;
//				}
//				break;
//				case 's':
//				{
//					arg_str = va_arg(arg_list, char*);
//				}
//				break;
//				default:
//				{
//					assert(0 && "Unsupported format");
//				}
//				break;
//			}
//			arg_str_len = strlen(arg_str);
//			if (fmtmsg_index + arg_str_len >= fmtmsg_len - 1)
//			{
//				arg_str_len = fmtmsg_len - fmtmsg_index - 1;
//				out = true;
//			}
//			memcpy(&fmtmsg[fmtmsg_index], arg_str, sizeof(char) * arg_str_len);
//			fmtmsg_index += arg_str_len;
//	//		fputs(arg_str, stdout);
//
//		}
//		else
//		{
////			putchar(*p);
//			fmtmsg[fmtmsg_index++] = *p;
//		}
//	}
//	fmtmsg[fmtmsg_index] = '\0';
//	va_end(arg_list);
//
//	return write_msg(severity, fmtmsg);
//}

unsigned short MsgDumperMgr::deinitialize()
{
	if (!is_init)
	{
		WRITE_ERR_SYSLOG("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper[i] != NULL)
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Release the Object: MsgDumper%s", dev_name[i]);
//			ret = msg_dumper[i]->deinitialize();
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				return ret;

			delete msg_dumper[i];
			msg_dumper[i] = NULL;
		}
	}

	is_init = false;

	return MSG_DUMPER_SUCCESS;
}

