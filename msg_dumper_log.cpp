#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "msg_dumper_log.h"


char* MsgDumperLog::DEF_LOG_FOLDER = "log";

MsgDumperLog::MsgDumperLog() :
	log_filename(NULL),
	log_filepath(NULL),
	fp_log(NULL)
{
	memcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_LOG], strlen(MSG_DUMPER_FACILITY_DESC[FACILITY_LOG]));
	memset(log_folder, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(log_folder, DEF_LOG_FOLDER, sizeof(char) * strlen(DEF_LOG_FOLDER));
}

MsgDumperLog::~MsgDumperLog()
{
	if (fp_log != NULL)
	{
		fclose(fp_log);
		fp_log = NULL;
	}

	if (log_filepath != NULL)
	{
		delete[] log_filepath;
		log_filepath = NULL;
	}

	if (log_filename != NULL)
	{
		delete[] log_filename;
		log_filename = NULL;
	}
//	deinitialize();
}

unsigned short MsgDumperLog::create_log_folder(const char* config_path)
{
	struct stat st = {0};

	char folder_path[MSG_DUMPER_LONG_STRING_SIZE];
	snprintf(folder_path, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s", config_path, log_folder);

// Check if the log folder exists or not
	if (stat(folder_path, &st) == -1)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to create a log folder: %s", folder_path);
// If not, create a new folder
		if (mkdir(folder_path, 0744) != 0)
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to create a log folder[%s], due to %s", folder_path, strerror(errno));
			return MSG_DUMPER_FAILURE_UNKNOWN;
		}
	}
	else
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "The log folder[%s] has already existed", folder_path);
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static char* title[] = {"log_folder"};
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
				param_member_variable = log_folder;
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

unsigned short MsgDumperLog::open_device()
{
// Open the file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the log file: %s", log_filename);
	fp_log = fopen(log_filepath, "a");
	if (fp_log == NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to open log file: %s", log_filename);
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::close_device()
{
// Close the file
	if (fp_log != NULL)
	{
		fclose(fp_log);
		fp_log = NULL;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::initialize(const char* config_path, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperLog object......");

// Parse the config file first
	unsigned short ret = parse_config(config_path, "log");
	if (CHECK_FAILURE(ret))
		return ret;

// Create the log folder
	ret = create_log_folder(config_path);
	if (CHECK_FAILURE(ret))
		return ret;

	if (log_filename != NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log file[%s] is NOT null", log_filename);
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	char current_time_string[CURRENT_TIME_STRING_LENGTH];
	generate_current_time_string(current_time_string);

// Determine the log file name
	log_filename = new char[MSG_DUMPER_SHORT_STRING_SIZE];
	log_filepath = new char[MSG_DUMPER_LONG_STRING_SIZE];
	if (log_filename == NULL || log_filepath == NULL)
	{
		WRITE_DEBUG_SYSLOG("Fail to allocate the memory: log_filename/log_filepath");
		return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
	}

	memset(log_filename, 0x0, sizeof(char) * MSG_DUMPER_SHORT_STRING_SIZE);
	snprintf(log_filename, MSG_DUMPER_SHORT_STRING_SIZE, "%s.log", current_time_string);
	memset(log_filepath, 0x0, sizeof(char) * MSG_DUMPER_SHORT_STRING_SIZE);
	char cwd[MSG_DUMPER_STRING_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		WRITE_DEBUG_SYSLOG("Fail to get the current working folder");
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
	snprintf(log_filepath, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s/%s", cwd, log_folder, log_filename);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Log file path: %s", log_filepath);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	WRITE_DEBUG_SYSLOG("DeInitialize the MsgDumperLog object......");

	return close_device();
}

unsigned short MsgDumperLog::write_msg(PMSG_CFG msg_cfg)
{
// Write the message into the log file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to file [%s]", msg_cfg->to_string(), log_filename);
	fputs(msg_cfg->to_string(), fp_log);

	return MSG_DUMPER_SUCCESS;
}
