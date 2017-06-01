#include <sys/stat.h>
#include "msg_dumper_base.h"
#include "msg_dumper.h"


using namespace std;

const char* MsgDumperBase::CONF_FOLDER_NAME = "conf";

MsgDumperBase::MsgDumperBase() :
	severity(MSG_DUMPER_SEVIRITY_DEBUG)
{
	memset(facility_name, 0x0, MSG_DUMPER_SHORT_STRING_SIZE);
}

unsigned short MsgDumperBase::generate_current_time_string(char* current_time_string)
{
	if (current_time_string ==  NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: current_time_string");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	time_t timep;
	time(&timep);
	struct tm* p = localtime(&timep);

	memset(current_time_string, 0x0, sizeof(char) * CURRENT_TIME_STRING_LENGTH);
	snprintf(current_time_string, CURRENT_TIME_STRING_LENGTH, "%02d%02d%02d%02d%02d", (p->tm_year + 1900) % 2000, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperBase::parse_config(const char* current_working_directory, const char* dev_name)
{
	if (current_working_directory == NULL || dev_name == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: current_working_directory/dev_name");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
// Open the file
	char conf_filepath[MSG_DUMPER_STRING_SIZE];
	snprintf(conf_filepath, MSG_DUMPER_STRING_SIZE, "%s/%s/dumper_param.conf", current_working_directory, CONF_FOLDER_NAME);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to parse the conf file: %s", conf_filepath);
	FILE* fp = fopen(conf_filepath, "r");
	if (fp == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to open the conf file: %s", conf_filepath);
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}

// Parse the content of the config file
	unsigned short ret = MSG_DUMPER_SUCCESS;
	char buf[MSG_DUMPER_LONG_STRING_SIZE];
	char start_flag[MSG_DUMPER_STRING_SIZE];
	snprintf(start_flag, MSG_DUMPER_STRING_SIZE, "[%s]", dev_name);
	int start_flag_len = strlen(start_flag);
	const char* stop_flag = "[";
	int stop_flag_len = strlen(stop_flag);
	bool param_start = false;
	while (fgets(buf, MSG_DUMPER_LONG_STRING_SIZE, fp) != NULL)
	{
		if (!param_start)
		{
			if (strncmp(buf, start_flag, start_flag_len) == 0)
				param_start = true;
			continue;
		}
		else
		{
			if (strncmp(buf, stop_flag, stop_flag_len) == 0)
				break;
		}
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Param content: %s", buf);
		int new_line_pos = -1;
		int split_pos = -1;
// Get the config for each line
		for (unsigned int i = 0 ; i < MSG_DUMPER_LONG_STRING_SIZE ; i++)
		{
			if (buf[i] == '\n')
			{
				buf[i] = '\0';
				new_line_pos = i;
				break;
			}
			if (buf[i] == '=')
				split_pos = i;
		}
		if (new_line_pos == 0)
			break;
		else if (new_line_pos < 0 || split_pos < 0)
		{
// Incorrect config! Fail to find the 'new line' character in a specific line
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Incorrect config: %s", buf);
			ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			break;
		}
// Update the parameter value
		string new_param(buf);
		ret = parse_config_param(new_param.substr(0, split_pos).c_str(), new_param.substr(split_pos + 1).c_str());
		if (CHECK_FAILURE(ret))
			break;
	}

// Close the file
	fclose(fp);
	fp = NULL;

	return ret;
}

unsigned short MsgDumperBase::set_severity(unsigned short new_severity)
{
	severity = new_severity;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperBase::get_severity()const
{
	return severity;
}

const char* MsgDumperBase::get_facility_name()const
{
	return facility_name;
}
