#include <unistd.h>
#include "msg_dumper_log.h"
#include "msg_dumper.h"
#include "common.h"


MsgDumperLog::MsgDumperLog() :
	exit(0)
{

}

void* MsgDumperLog::msg_dumper_log_handler(void* void_ptr)
{
	if (void_ptr != NULL)
	{
		MsgDumperLog* this_ptr = (MsgDumperLog*)void_ptr;
		if (CHECK_MSG_DUMPER_FAILURE(this_ptr->msg_dumper_log_handler_internal()))
		{
			pthread_exit((void*)"Failure");
		}
	}

	pthread_exit((void*)"Success");
}

unsigned short MsgDumperLog::msg_dumper_log_handler_internal()
{
	WRITE_DEBUG_SYSLOG("The worker thread of writing message is running");
	while (!exit)
	{
		WRITE_DEBUG_SYSLOG("Run!");
		sleep(1);
	}

	WRITE_DEBUG_SYSLOG("The worker thread of writing message is going to die");
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::initialize(void* config)
{
	int api_ret;
	api_ret = pthread_create(&pid, NULL, msg_dumper_log_handler, this);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_create() failed, due to: %d", api_ret);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_msg(const char* msg)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	__sync_lock_test_and_set(&exit, 1);

	int api_ret;
	char api_ret_desc[256];
	api_ret = pthread_join(pid, (void**)&api_ret_desc);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_join() failed, due to: %s", api_ret_desc);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}

	return MSG_DUMPER_SUCCESS;
}
