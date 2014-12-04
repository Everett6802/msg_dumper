#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_remote.h"


MsgDumperRemote::MsgDumperRemote()
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "REMOTE");
}

MsgDumperRemote::~MsgDumperRemote()
{

//	deinitialize();
}

unsigned short MsgDumperRemote::create_device_file()
{

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::write_device_file()
{

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::parse_config_param(const char* param_title, const char* param_content)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::initialize(void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperRemote object......");

// Parse the config file first
	unsigned short ret = parse_config("remote");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	return MsgDumperTimerThread::initialize(config);
}
