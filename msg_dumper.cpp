#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"


static MsgDumperMgr msg_dumepr_mgr;

unsigned short msg_dumper_initialize()
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.initialize();
}

unsigned short msg_dumper_get_version(unsigned short& major_version, unsigned char& minor_version)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	major_version = MAJOR_VERSION;
	minor_version = MINOR_VERSION;

	return MSG_DUMPER_SUCCESS;
}

unsigned short msg_dumper_set_severity(unsigned short severity)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
//	return MSG_DUMPER_SUCCESS;
	return msg_dumepr_mgr.set_severity(severity);
}

unsigned short msg_dumper_set_facility(unsigned short facility)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
//	return MSG_DUMPER_SUCCESS;
	return msg_dumepr_mgr.set_facility(facility);
}

unsigned short msg_dumper_write_msg(unsigned short severity, const char* msg)
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.write_msg(severity, msg);
}

unsigned short msg_dumper_deinitialize()
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "%s() called", __func__);
	return msg_dumepr_mgr.deinitialize();
}

