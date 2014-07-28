#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"


MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_severity(MSG_DUMPER_SEVIRITY_ERROR),
	dumper_facility(MSG_DUMPER_FACILITY_ALL)
{

}

unsigned short MsgDumperMgr::initialize()
{
	is_init = true;
	return MSG_DUMPER_SUCCESS;
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

	if (severity <= dumper_severity)
	{

	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::deinitialize()
{
	is_init = false;
	return MSG_DUMPER_SUCCESS;
}

