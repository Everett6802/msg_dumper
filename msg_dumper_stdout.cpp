#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>
#include "msg_dumper.h"
#include "msg_dumper_stdout.h"


using namespace std;

enum OutputType{OUTPUT_TYPE_IONBF, OUTPUT_TYPE_IOLBF, OUTPUT_TYPE_IOFBF, OUTPUT_TYPE_SIZE};
static int output_type_value[OUTPUT_TYPE_SIZE] = {_IONBF, _IOLBF, _IOFBF};

const char* MsgDumperStdout::MSG_DUMPER_TITLE = "MsgDumper";
// const char* MsgDumperStdout::DEF_SYSLOG_FACILITY_NAME = "daemon";

MsgDumperStdout::MsgDumperStdout() :
	normal_output_type(OUTPUT_TYPE_IOLBF),
	error_output_type(OUTPUT_TYPE_IONBF)
{
	// memcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_STDOUT], strlen(MSG_DUMPER_FACILITY_DESC[FACILITY_STDOUT]));
	strcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_STDOUT]);
}

MsgDumperStdout::~MsgDumperStdout()
{
}

unsigned short MsgDumperStdout::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERROR("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static const char* title[] = {"normal_output", "error_output"};
	static int title_len = sizeof title / sizeof title[0];

	unsigned short ret = MSG_DUMPER_SUCCESS;
	bool found = false;
	for (int index = 0 ; index < title_len ; index++)
	{
		if (strcmp(title[index], param_title) == 0)
		{
			switch(index)
			{
			case 0:
			{
				normal_output_type = atoi(param_content);
				if (normal_output_type < 0 || normal_output_type >= OUTPUT_TYPE_SIZE)
				{
					WRITE_ERROR("Incorrect parameter, unknown normal output type: %s", param_content);
					return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
				}
			}
			break;
			case 1:
			{
				error_output_type = atoi(param_content);
				if (error_output_type < 0 || error_output_type >= OUTPUT_TYPE_SIZE)
				{
					WRITE_ERROR("Incorrect parameter, unknown error output type: %s", param_content);
					return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
				}
			}
			break;
			default:
			{
				return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			}
			break;
			}
			found = true;
		}
		if (found)
			break;
	}
// If the title is NOT found...
	if (!found)
	{
		WRITE_ERROR("Incorrect parameter, fail to find the title: %s", param_title);
		ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	return ret;
}

unsigned short MsgDumperStdout::open_device(){return MSG_DUMPER_SUCCESS;}

unsigned short MsgDumperStdout::close_device(){return MSG_DUMPER_SUCCESS;}

unsigned short MsgDumperStdout::initialize(const char* current_working_directory, void* config)
{
	WRITE_DEBUG("Initialize the MsgDumperStdout object......");
// Parse the config file first
	unsigned short ret = parse_config(current_working_directory, "stdout");
	if (CHECK_FAILURE(ret))
		return ret;
// STDOUT default: line buffer
    setvbuf(stdout, NULL, output_type_value[normal_output_type], 0);
// STDERR default: error immediate
    setvbuf(stderr, NULL, output_type_value[error_output_type], 0);
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperStdout::deinitialize()
{
	// WRITE_DEBUG("DeInitialize the MsgDumperStdout object......");
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperStdout::write_msg(PMSG_CFG msg_cfg)
{
	if (msg_cfg == NULL)
		throw invalid_argument("msg_cfg should NOT be NULL");
	switch(msg_cfg->msg_dumper_severity)
	{
		case MSG_DUMPER_SEVERITY_ERROR:
		{
			fprintf(stderr, "%s\n", msg_cfg->to_string());
		}
		break;
		default:
		{
			printf("%s\n", msg_cfg->to_string());
		}
		break;
	}
    
	return MSG_DUMPER_SUCCESS;
}
