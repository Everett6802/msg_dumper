#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <stdexcept>
#include "msg_dumper_wrapper.h"


using namespace std;

#define CHECK_FAILURE(x) ((x != MSG_DUMPER_SUCCESS) ? true : false)
//#define DO_DEBUG

MsgDumperWrapper* MsgDumperWrapper::instance = NULL;
const char* MsgDumperWrapper::MSG_DUMPER_ERROR_COLOR = "\x1B[31m";
const char* MsgDumperWrapper::FACILITY_NAME[] = {"Log", "Com", "Sql", "Remote", "Syslog"};
const int MsgDumperWrapper::FACILITY_NAME_SIZE = sizeof(FACILITY_NAME) / sizeof(FACILITY_NAME[0]);
const unsigned short MsgDumperWrapper::FACILITY_FLAG[] = {MSG_DUMPER_FACILITY_LOG, MSG_DUMPER_FACILITY_COM, MSG_DUMPER_FACILITY_SQL, MSG_DUMPER_FACILITY_REMOTE, MSG_DUMPER_FACILITY_SYSLOG};
const char* MsgDumperWrapper::SEVERITY_NAME[] = {"ERROR", "WARN", "INFO", "DEBUG"};
const int MsgDumperWrapper::SEVERITY_NAME_SIZE = sizeof(SEVERITY_NAME) / sizeof(SEVERITY_NAME[0]);
const char* MsgDumperWrapper::CONF_FOLDER = "conf";
const char* MsgDumperWrapper::CONF_FILENAME = "dumper_param.conf";

MsgDumperWrapper::MsgDumperWrapper() :
	ref_count(0),
	api_handle(NULL),
	fp_msg_dumper_initialize(NULL),
	fp_msg_dumper_get_version(NULL),
	fp_msg_dumper_set_severity(NULL),
	fp_msg_dumper_set_facility(NULL),
	fp_msg_dumper_get_severity(NULL),
	fp_msg_dumper_get_facility(NULL),
	fp_msg_dumper_write_msg(NULL),
	fp_msg_dumper_deinitialize(NULL),
	fp_msg_dumper_get_error_description(NULL)
{
}

bool MsgDumperWrapper::export_api()
{
	fp_msg_dumper_get_version = (FP_msg_dumper_get_version)dlsym(api_handle, "msg_dumper_get_version");
	if (fp_msg_dumper_get_version == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_get_version() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_initialize = (FP_msg_dumper_initialize)dlsym(api_handle, "msg_dumper_initialize");
	if (fp_msg_dumper_initialize == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_initialize() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_set_severity = (FP_msg_dumper_set_severity)dlsym(api_handle, "msg_dumper_set_severity");
	if (fp_msg_dumper_set_severity == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_set_severity() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_set_facility = (FP_msg_dumper_set_facility)dlsym(api_handle, "msg_dumper_set_facility");
	if (fp_msg_dumper_set_facility == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_set_facility() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_get_severity = (FP_msg_dumper_get_severity)dlsym(api_handle, "msg_dumper_get_severity");
	if (fp_msg_dumper_get_severity == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_get_severity() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_get_facility = (FP_msg_dumper_get_facility)dlsym(api_handle, "msg_dumper_get_facility");
	if (fp_msg_dumper_get_facility == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_get_facility() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_write_msg = (FP_msg_dumper_write_msg)dlsym(api_handle, "msg_dumper_write_msg");
	if (fp_msg_dumper_write_msg == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_write_msg() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_deinitialize = (FP_msg_dumper_deinitialize)dlsym(api_handle, "msg_dumper_deinitialize");
	if (fp_msg_dumper_deinitialize == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_deinitialize() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}
	fp_msg_dumper_get_error_description = (FP_msg_dumper_get_error_description)dlsym(api_handle, "msg_dumper_get_error_descriptions");
	if (fp_msg_dumper_deinitialize == NULL)
	{
		fprintf(stderr, "%sdlsym() fails when exporting msg_dumper_get_error_description() due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return false;
	}

	return true;
}

unsigned short MsgDumperWrapper::initialize()
{
// Load library
	unsigned short ret = MSG_DUMPER_SUCCESS;
	api_handle = dlopen("libmsg_dumper.so", RTLD_NOW);
	if (api_handle == NULL)
	{
		fprintf(stderr, "%sdlopen() fails, due to %s\n", MSG_DUMPER_ERROR_COLOR, dlerror());
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}

// Export the APIs
	if (!export_api())
	{
		fprintf(stderr, "%sFail to export the APIs\n", MSG_DUMPER_ERROR_COLOR);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}

	unsigned char major_version;
	unsigned char minor_version;
	unsigned char build_version;
	fp_msg_dumper_get_version(major_version, minor_version, build_version);
#ifdef DO_DEBUG
	printf("API version: (%d.%d.%d)\n", major_version, minor_version, build_version);
#endif

// Parse the parameters from the config file
//	printf("Parse the config file\n");
	ret = parse_config();
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "%sfparse_config() fails, due to %d\n", MSG_DUMPER_ERROR_COLOR, ret);
		return ret;
	}

// Initialize the library
#ifdef DO_DEBUG
	printf("Initialize the library\n");
#endif
	ret = fp_msg_dumper_initialize();
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "%sfp_msg_dumper_initialize() fails, due to %d\n", MSG_DUMPER_ERROR_COLOR, ret);
		return ret;
	}

	return ret;
}

void MsgDumperWrapper::deinitialize()
{
// De-initialize the library
#ifdef DO_DEBUG
	printf("Close the library\n");
#endif
	fp_msg_dumper_deinitialize();

// Close the handle
	if (api_handle != NULL)
	{
         instance = NULL;
		dlclose(api_handle);
		api_handle = NULL;
	}
}

int MsgDumperWrapper::get_severity_index(const char* severity)const
{
	assert(severity != NULL && "severity should NOT be NULL");
	int severity_index = -1;
	for (int i = 0 ; i < SEVERITY_NAME_SIZE ; i++)
	{
		// fprintf(stderr, "%s, %s, %d\n", severity, SEVERITY_NAME[i], strcmp(severity, SEVERITY_NAME[i]));
		if (strcmp(severity, SEVERITY_NAME[i]) == 0)
		{
			severity_index = i;
			break;
		}
	}
	// if (severity_index == -1)
	// {
	// 	char exception[64];
	// 	snprintf("Unknown severity name: %s", 64, severity_name);
	// 	throw invalid_argument(string(exception));
	// }
	return severity_index;
}

int MsgDumperWrapper::get_facility_index(const char* facility)const
{
	assert(facility != NULL && "facility should NOT be NULL");
	int facility_index = -1;
	for (int i = 0 ; i < FACILITY_NAME_SIZE ; i++)
	{
		if (strcmp(facility, FACILITY_NAME[i]) == 0)
		{
			facility_index = i;
			break;
		}
	}
	// if (facility_index == -1)
	// {
	// 	char exception[64];
	// 	snprintf("Unknown facility name: %s", 64, facility_name);
	// 	throw invalid_argument(string(exception));
	// }
	return facility_index;
}

unsigned short MsgDumperWrapper::parse_config()
{
	static const int BUF_SIZE = 256;
	char current_working_directory[BUF_SIZE];
	getcwd(current_working_directory, sizeof(current_working_directory));
	char config_filename[BUF_SIZE];
	snprintf(config_filename, BUF_SIZE, "%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
#ifdef DO_DEBUG
	printf("Parse the config file: %s\n", config_filename);
#endif

	FILE *fp = fopen(config_filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%sfopen() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, strerror(errno));
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}

	unsigned short total_facility_flag = 0x0;
	unsigned short ret = MSG_DUMPER_SUCCESS;
// Parse the config file
	const char* stop_flag = "[";
	int stop_flag_len = strlen(stop_flag);
	char buf[BUF_SIZE];

	while (fgets(buf, BUF_SIZE, fp) != NULL)
	{
		if (strncmp(buf, stop_flag, stop_flag_len) == 0)
			break;

		if (buf[0] == '\n')
			break;
		int end_pos = -1;
		int split_pos = -1;
// Check the format of config entry is correct
		for (int i = 0 ; i < BUF_SIZE ; i++)
		{
			if (buf[i] == '=')
				split_pos = i;
			else if (buf[i] == '\n')
			{
				end_pos = i;
				buf[i] = '\0';
				break;
			}
		}
		if (end_pos == -1 || split_pos == -1)
		{
			fprintf(stderr, "%sIncorrect config format, split pos: %d, end pos: %d\n", MSG_DUMPER_ERROR_COLOR, split_pos, end_pos);
			ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
			break;
		}

		string config(buf);
//		WRITE_DEBUG_FORMAT_LOGGING(STRING_SIZE, "***Config*** content: %s, split pos: %d", config.c_str(), split_pos);
		char facility[16];
		snprintf(facility, 16, "%s", config.substr(0, split_pos).c_str());
		char severity[16];
		snprintf(severity, 16, "%s", config.substr(split_pos + 1).c_str());
#ifdef DO_DEBUG
		printf("***Config***\nfacility: %s, severity: %s\n", facility, severity);
#endif
//		unsigned short facility_flag;
		int facility_index = get_facility_index(facility);
// Set facility
		// for (int i = 0 ; i < FACILITY_NAME_SIZE ; i++)
		// {
		// 	if (strcmp(facility, FACILITY_NAME[i]) == 0)
		// 	{
		// 		facility_index = i;
		// 		break;
		// 	}
		// }
		if (facility_index == -1)
		{
			fprintf(stderr, "%sUnknown facility: %s\n", MSG_DUMPER_ERROR_COLOR, facility);
			ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
			goto OUT;
		}
		total_facility_flag |= FACILITY_FLAG[facility_index];
// Set severity
		int severity_index = get_severity_index(severity);
		if (severity_index == -1)
		{
			fprintf(stderr, "%sUnknown severity: %s\n", MSG_DUMPER_ERROR_COLOR, severity);
			ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
			goto OUT;
		}
#ifdef DO_DEBUG
		printf("Set severity of facility[%s] to %s\n", FACILITY_NAME[facility_index], SEVERITY_NAME[severity_index]);
#endif
		ret = fp_msg_dumper_set_severity(severity_index, FACILITY_FLAG[facility_index]);
		if (CHECK_FAILURE(ret))
		{
			fprintf(stderr, "%sfp_msg_dumper_set_severity() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());
			goto OUT;
		}
// 		for (int i = 0 ; i < SEVERITY_NAME_SIZE ; i++)
// 		{
// 			if (strcmp(severity, SEVERITY_NAME[i]) == 0)
// 			{
// // Assign the value in the config to the member variable
// 				switch(i)
// 				{
// 					case MSG_DUMPER_SEVIRITY_ERROR:
// 					case MSG_DUMPER_SEVIRITY_WARN:
// 					case MSG_DUMPER_SEVIRITY_INFO:
// 					case MSG_DUMPER_SEVIRITY_DEBUG:
// 					{
// #ifdef DO_DEBUG
// 						printf("Set severity of facility[%s] to %s\n", FACILITY_NAME[facility_index], SEVERITY_NAME[i]);
// #endif
// 						ret = fp_msg_dumper_set_severity(i, FACILITY_FLAG[facility_index]);
// 						if (CHECK_FAILURE(ret))
// 						{
// 							fprintf(stderr, "%sfp_msg_dumper_set_severity() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());
// 							goto OUT;
// 						}
// 					}
// 					break;
// 					default:
// 					{
// 						fprintf(stderr, "%sUnknown severity: %s\n", MSG_DUMPER_ERROR_COLOR, severity);
// 						ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
// 						goto OUT;
// 					}
// 					break;
// 				}
// 			}
// 		}
	}
#ifdef DO_DEBUG
	printf("Set facility to %d\n", total_facility_flag);
#endif
	ret = fp_msg_dumper_set_facility(total_facility_flag);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "%sfp_msg_dumper_set_facility() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());
		return ret;
	}

OUT:
	fclose(fp);
	fp = NULL;

	return ret;
}

MsgDumperWrapper* MsgDumperWrapper::get_instance()
{
	if (instance == NULL)
	{
// If the instance is NOT created...
		instance = new MsgDumperWrapper();
		if (instance == NULL)
		{
			assert(0 || "Fail to get the instance of MsgDumperWrapper");
			return NULL;
		}

// Initialize the instance
		unsigned short ret = instance->initialize();
		if(CHECK_FAILURE(ret))
		{
			assert(0 || "Fail to get the instance of MsgDumperWrapper");
			return NULL;
		}
	}

// Add the reference count
	instance->addref();
	return instance;
}

int MsgDumperWrapper::addref()
{
	__sync_fetch_and_add(&ref_count, 1);
	return ref_count;
}

int MsgDumperWrapper::release()
{
	__sync_fetch_and_sub(&ref_count, 1);
	if (ref_count == 0)
	{
		delete this;
		return 0;
	}

	return ref_count;
}

unsigned short MsgDumperWrapper::set_severity(unsigned short severity_index, unsigned short facility_flag)
{
	unsigned short ret = fp_msg_dumper_set_severity(severity_index, facility_flag);
	if (CHECK_FAILURE(ret))
		fprintf(stderr, "%sfp_msg_dumper_set_severity() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperWrapper::get_severity(unsigned short facility_flag)const
{
	unsigned short ret = fp_msg_dumper_get_severity(facility_flag);
	if (CHECK_FAILURE(ret))
		fprintf(stderr, "%sfp_msg_dumper_get_severity() fails, reason: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperWrapper::set_log_severity(unsigned short log_severity_index)
{
	return set_severity(log_severity_index, MSG_DUMPER_FACILITY_LOG);
}

unsigned short MsgDumperWrapper::set_syslog_severity(unsigned short syslog_severity_index)
{
	return set_severity(syslog_severity_index, MSG_DUMPER_FACILITY_SYSLOG);
}

unsigned short MsgDumperWrapper::get_log_severity()const
{
	return get_severity(MSG_DUMPER_FACILITY_LOG);
}

unsigned short MsgDumperWrapper::get_syslog_severity()const
{
	return get_severity(MSG_DUMPER_FACILITY_SYSLOG);
}

unsigned short MsgDumperWrapper::set_config(const char* config_name, const char* config_value)
{
	assert(config_name != NULL && "config_name should NOT be NULL");
	assert(config_value != NULL && "config_value should NOT be NULL");
	static const int FILE_PATH_SIZE = 256;
	static const char* COMMAND_FORMAT = "sed -i 's/^%s=.*/%s=%s/g' %s";
	static const int COMMAND_SIZE = 512;
	char current_working_directory[FILE_PATH_SIZE];
	getcwd(current_working_directory, FILE_PATH_SIZE);
	char conf_filepath[FILE_PATH_SIZE];
	snprintf(conf_filepath, FILE_PATH_SIZE, "%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
	char command[COMMAND_SIZE];
	snprintf(command, COMMAND_SIZE, COMMAND_FORMAT, config_name, config_name, config_value, conf_filepath);
	// fprintf(stderr, "Command: %s\n", command);
	FILE* fp_set = popen(command, "w");
	if (fp_set == NULL)
	{
		fprintf(stderr, "%spopen() fails, due to: %s\n", MSG_DUMPER_ERROR_COLOR, strerror(errno));
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}
	pclose(fp_set);
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperWrapper::get_config(const char* config_name, char* config_value)const
{
	assert(config_name != NULL && "config_name should NOT be NULL");
	assert(config_value != NULL && "config_value should NOT be NULL");

	static const char* COMMAND_FORMAT = "cat %s | grep '%s='";
	static const int FILE_PATH_SIZE = 256;
	static const int COMMAND_SIZE = 512;
	static const int LINE_SIZE = 64;
	char current_working_directory[FILE_PATH_SIZE];
	getcwd(current_working_directory, FILE_PATH_SIZE);
	char conf_filepath[FILE_PATH_SIZE];
	snprintf(conf_filepath, FILE_PATH_SIZE, "%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
	char command[COMMAND_SIZE];
	snprintf(command, COMMAND_SIZE, COMMAND_FORMAT, conf_filepath, config_name);
	FILE* fp_get = popen(command, "r");
	if (fp_get == NULL)
	{
		fprintf(stderr, "%spopen() fails, due to: %s\n", MSG_DUMPER_ERROR_COLOR, strerror(errno));
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}
	unsigned short ret = MSG_DUMPER_SUCCESS;
	char line[LINE_SIZE];
	char* buf = line;
	char *line_config_name = NULL, *line_config_value = NULL;
	if (fgets(line, LINE_SIZE, fp_get) == NULL)
	{
		fprintf(stderr, "%sfgets() fails, due to: %s\n", MSG_DUMPER_ERROR_COLOR, strerror(errno));
		ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
		goto OUT;
	}
	line_config_name = strtok(buf, "=");
	line_config_value = strtok(NULL, "\t\n\r");
	if (line_config_name == NULL || line_config_value == NULL)
	{
		fprintf(stderr, "%sIncorrect config: %s\n", MSG_DUMPER_ERROR_COLOR, line);
		ret = MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
		goto OUT;
	}
	// printf("*** Parsing *** Line: %s, Name: %s, Value: %s\n", line, line_config_name, line_config_value);
	memcpy(config_value, line_config_value, strlen(line_config_value) + 1);
	// printf("*** Result *** Value: %s\n", config_value);
OUT:
	pclose(fp_get);
	return ret;
}

unsigned short MsgDumperWrapper::set_log_severity_config(unsigned short log_severity)
{
	if (log_severity < 0 || log_severity >= SEVERITY_NAME_SIZE)
	{
		fprintf(stderr, "%sThe log severity[%d] is Out of Range[0, %d)\n", MSG_DUMPER_ERROR_COLOR, log_severity, SEVERITY_NAME_SIZE);
		return MSG_DUMPER_FAILURE_OUT_OF_RANGE;
	}
	return set_config("Log", SEVERITY_NAME[log_severity]);
}

unsigned short MsgDumperWrapper::set_syslog_severity_config(unsigned short syslog_severity)
{
	if (syslog_severity < 0 || syslog_severity >= SEVERITY_NAME_SIZE)
	{
		fprintf(stderr, "%sThe syslog severity[%d] is Out of Range[0, %d)\n", MSG_DUMPER_ERROR_COLOR, syslog_severity, SEVERITY_NAME_SIZE);
		return MSG_DUMPER_FAILURE_OUT_OF_RANGE;
	}
	return set_config("Syslog", SEVERITY_NAME[syslog_severity]);
}

unsigned short MsgDumperWrapper::get_log_severity_config()const
{
	static const int SEVERITY_STRING_SIZE = 16;
	static char severity[SEVERITY_STRING_SIZE];
	unsigned short ret = get_config("Log", severity);
	if (CHECK_FAILURE(ret))
		return ret;
	int severity_index = get_severity_index(severity);
	if (severity_index == -1)
	{
		char exception[64];
		snprintf(exception, 64,"Unknown severity: %s", severity);
		throw invalid_argument(string(exception));
	}
	return severity_index;
}

unsigned short MsgDumperWrapper::get_syslog_severity_config()const
{
	static const int SEVERITY_STRING_SIZE = 16;
	static char severity[SEVERITY_STRING_SIZE];
	unsigned short ret = get_config("Syslog", severity);
	if (CHECK_FAILURE(ret))
		return ret;
	// fprintf(stderr, "severity: %s\n", severity);
	int severity_index = get_severity_index(severity);
	if (severity_index == -1)
	{
		char exception[64];
		snprintf(exception, 64,"Unknown severity: %s", severity);
		throw invalid_argument(string(exception));
	}
	return severity_index;
}

unsigned short MsgDumperWrapper::write(unsigned short syslog_priority, const char* msg)
{
#if 0
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#endif

	unsigned short msg_severity;
	switch(syslog_priority)
	{
	case LOG_DEBUG:
		msg_severity = MSG_DUMPER_SEVIRITY_DEBUG;
		break;
	case LOG_INFO:
		msg_severity = MSG_DUMPER_SEVIRITY_INFO;
		break;
	case LOG_WARNING:
	case LOG_NOTICE:
		msg_severity = MSG_DUMPER_SEVIRITY_WARN;
		break;
	default:
		msg_severity = MSG_DUMPER_SEVIRITY_ERROR;
		break;
	}

	unsigned short ret = fp_msg_dumper_write_msg(msg_severity, msg);
	if (CHECK_FAILURE(ret))
		fprintf(stderr, "%sfp_msg_dumper_write_msg() fails, resaon: %s\n", MSG_DUMPER_ERROR_COLOR, fp_msg_dumper_get_error_description());

	return ret;
}

const char* MsgDumperWrapper::get_error_description()const
{
	return fp_msg_dumper_get_error_description();
}
