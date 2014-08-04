#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"


MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_severity(MSG_DUMPER_SEVIRITY_DEBUG),
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

	unsigned short ret = MSG_DUMPER_SUCCESS;
// Allocate and initialize the MsgDumplerLog object
	if (dumper_facility & MSG_DUMPER_FACILITY_LOG)
	{
		WRITE_DEBUG_SYSLOG("Allocate the MsgDumperLog object");
		msg_dumper[FACILITY_LOG] = new MsgDumperLog();
		if (msg_dumper[FACILITY_LOG] == NULL)
		{
			WRITE_ERR_SYSLOG("Fail to allocate the MsgDumperLog object");
			return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
		}

		WRITE_DEBUG_SYSLOG("Initialize the MsgDumperLog object");
		ret = msg_dumper[FACILITY_LOG]->initialize();
		if (CHECK_MSG_DUMPER_FAILURE(ret))
		{
			WRITE_ERR_SYSLOG("Fail to initialize the MsgDumperLog object");
			goto EXIT;
		}
	}
// Allocate and initialize the MsgDumplerCom object
	if (dumper_facility & MSG_DUMPER_FACILITY_COM)
	{
		WRITE_DEBUG_SYSLOG("Allocate the MsgDumperCom object");
		msg_dumper[FACILITY_COM] = new MsgDumperCom();
		if (msg_dumper[FACILITY_COM] == NULL)
		{
			WRITE_ERR_SYSLOG("Fail to allocate the MsgDumperCom object");
			return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
		}

		WRITE_DEBUG_SYSLOG("Initialize the MsgDumperCom object");
		ret = msg_dumper[FACILITY_COM]->initialize();
		if (CHECK_MSG_DUMPER_FAILURE(ret))
		{
			WRITE_ERR_SYSLOG("Fail to initialize the MsgDumperCom object");
			goto EXIT;
		}
	}

	is_init = true;
EXIT:
	return ret;
}

unsigned short MsgDumperMgr::set_severity(unsigned short severity)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	dumper_severity = severity;
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity: %s", dumper_severity);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::set_facility(unsigned short facility)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	dumper_facility = facility;
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the facility: %s", dumper_facility);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::write_msg(unsigned short severity, const char* msg)
{
	if (msg == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid pointer: msg");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	static char* dev_name[] = {"Log", "Com"};

	unsigned short ret = MSG_DUMPER_SUCCESS;
	if (severity <= dumper_severity)
	{
		for (int i = 0 ; i < FACILITY_SIZE ; i++)
		{
			if (msg_dumper[i])
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [%s] to %s", msg, dev_name[i]);
				ret = msg_dumper[i]->write_msg(msg);
				if (CHECK_MSG_DUMPER_FAILURE(ret))
					return ret;
			}
		}
	}

	return ret;
}

unsigned short MsgDumperMgr::deinitialize()
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	for (int i = 0 ; i < sizeof(msg_dumper) / sizeof(msg_dumper[0]) ; i++)
	{
		if (msg_dumper[i] != NULL)
		{
			ret = msg_dumper[i]->deinitialize();
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				return ret;

			delete msg_dumper[i];
			msg_dumper[i] = NULL;
		}
	}

	is_init = false;

	return MSG_DUMPER_SUCCESS;
}

