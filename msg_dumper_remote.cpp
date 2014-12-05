#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_remote.h"


MsgDumperRemote::MsgDumperRemote()
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "REMOTE");
	memset(server_list, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
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
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static char* title[] = {"server_list"};
	static int title_len = sizeof title / sizeof title[0];

	unsigned short ret = MSG_DUMPER_SUCCESS;
	bool found = false;
	for (int index = 0 ; index < title_len ; index++)
	{
		if (strcmp(title[index], param_title) == 0)
		{
			int param_content_len = strlen(param_content);
			char* param_member_variable = NULL;
			switch(index)
			{
			case 0:
				param_member_variable = server_list;
				break;
			}

			if (param_member_variable != NULL)
			{
				memset(param_member_variable, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
				memcpy(param_member_variable, param_content, param_content_len);
				found = true;
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Update parameter: %s=%s", param_title, param_content);
			}
			else
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter: %s=%s", param_title, param_content);
				ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			}
			break;
		}
	}
// If the title is NOT found...
	if (!found)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter, fail to find the title: %s", param_title);
		ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	return ret;

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
