#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"
#include "msg_dumper_sql.h"
#include "msg_dumper_remote.h"


char* MsgDumperMgr::dev_name[] = {"Log", "Com", "Sql", "Remote"};
short MsgDumperMgr::dev_flag[] = {MSG_DUMPER_FACILITY_LOG, MSG_DUMPER_FACILITY_COM, MSG_DUMPER_FACILITY_SQL, MSG_DUMPER_FACILITY_REMOTE};

MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_severity(MSG_DUMPER_SEVIRITY_ERROR),
	dumper_facility(MSG_DUMPER_FACILITY_LOG)
{
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		msg_dumper[i] = NULL;
}

unsigned short MsgDumperMgr::initialize()
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

// Register the class to the simple factory
	REGISTER_CLASS(MsgDumperLog);
	REGISTER_CLASS(MsgDumperCom);
	REGISTER_CLASS(MsgDumperSql);
	REGISTER_CLASS(MsgDumperRemote);

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
			ret = msg_dumper[i]->initialize();
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
	if (severity <= dumper_severity)
	{
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
	}

	return ret;
}

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

