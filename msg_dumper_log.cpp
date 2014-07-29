#include "msg_dumper_log.h"
#include "msg_dumper.h"

unsigned short MsgDumperLog::initialize(void* config)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_msg(const char* msg)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	return MSG_DUMPER_SUCCESS;
}
