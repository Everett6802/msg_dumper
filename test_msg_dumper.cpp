#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <string>
#include "common.h"
#include "msg_dumper.h"


// #define CHECK_FAILURE(x) ((x != MSG_DUMPER_SUCCESS) ? true : false)

using namespace std;

const char* CONF_FOLDER = "conf";
const char* CONF_FILENAME = "dumper_param.conf";

void* api_handle;
FP_msg_dumper_get_version fp_msg_dumper_get_version;
FP_msg_dumper_set_severity_index fp_msg_dumper_set_severity_index;
FP_msg_dumper_set_severity fp_msg_dumper_set_severity;
// FP_msg_dumper_set_facility fp_msg_dumper_set_facility;
FP_msg_dumper_get_severity_index fp_msg_dumper_get_severity_index;
FP_msg_dumper_get_severity fp_msg_dumper_get_severity;
// FP_msg_dumper_get_facility fp_msg_dumper_get_facility;
FP_msg_dumper_initialize fp_msg_dumper_initialize;
FP_msg_dumper_write_msg fp_msg_dumper_write_msg;
FP_msg_dumper_write_format_msg fp_msg_dumper_write_format_msg;
FP_msg_dumper_deinitialize fp_msg_dumper_deinitialize;
FP_msg_dumper_get_error_description fp_msg_dumper_get_error_description;

bool export_api();
unsigned short parse_config();
// bool test();
// bool test_macro();

int main()
{
// Load library
	unsigned short ret = MSG_DUMPER_SUCCESS;
	unsigned char major_version, minor_version, build_version;

	api_handle = dlopen("libmsg_dumper.so", RTLD_NOW);
	if (api_handle == NULL)
	{
		fprintf(stderr, "dlopen() fails, due to %s\n", dlerror());
		goto OUT;
	}
// Export API
	if (!export_api())
		goto OUT;

// Get API version
	fp_msg_dumper_get_version(&major_version, &minor_version, &build_version);
	printf("API version: (%d.%d.%d)\n", major_version, minor_version, build_version);
// Parse the parameters from the config file
//	printf("Parse the config file\n");
	ret = parse_config();
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "parse_config() fails, due to %d\n", ret);
		goto OUT;
	}
// Testing
// Get the severity of the facility
	int severity_index;
	for (int i = 0; i < MSG_DUMPER_FACILITY_DESC_LEN; i++)
	{
		ret = fp_msg_dumper_get_severity_index(MSG_DUMPER_FACILITY_DESC[i], &severity_index);
		if (CHECK_FAILURE(ret))
		{	
			fprintf(stderr, "fp_msg_dumper_get_severity(%s) fails, due to %d\n", MSG_DUMPER_FACILITY_DESC[i], ret);
			goto OUT;
		}
		if (severity_index == MSG_DUMPER_SEVERITY_NOSET)
			printf("Facility: %s, Severity: %s\n", MSG_DUMPER_FACILITY_DESC[i], MSG_DUMPER_SEVERITY_DESC_NOSET);
		else
			printf("Facility: %s, Severity: %s\n", MSG_DUMPER_FACILITY_DESC[i], MSG_DUMPER_SEVERITY_DESC[severity_index]);
	}
// Initialize the library
	ret = fp_msg_dumper_initialize();
	if (CHECK_FAILURE(ret))
	{	
		fprintf(stderr, "fp_msg_dumper_initialize() fails, due to %d\n", ret);
		goto OUT;
	}
// Dump strings
	fprintf(stderr, "Start to dump strings...\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_INFO, "This is a test info message");
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_INFO) fails, due to %d\n", ret);
		goto OUT1;
	}
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_WARN, "This is a test warn message");
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_WARN) fails, due to %d\n", ret);
		goto OUT1;
	}
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_ERROR, "This is a test error message");
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg(MSG_DUMPER_SEVERITY_ERROR) fails, due to %d\n", ret);
		goto OUT1;
	}
// Dump format strings
	fprintf(stderr, "Start to dump format strings...\n");
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_INFO, "This is a test info message with format: %d", 42);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_INFO) fails, due to %d\n", ret);
		goto OUT1;
	}
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_WARN, "This is a test warn message with format: %s", "example");
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_WARN) fails, due to %d\n", ret);
		goto OUT1;
	}
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_ERROR, "This is a test error message with format: %f", 3.14);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVERITY_ERROR) fails, due to %d\n", ret);
		goto OUT1;
	}
OUT1:
// De-initialize the library
	fp_msg_dumper_deinitialize();
OUT:
// 在還有 thread 在跑的情況下呼叫 dlclose()（或 deinitialize），導致 .so 被 unload，但 thread 還在執行裡面的 code
// thread 還在跑 -> object memory 被 free -> 甚至 .so 被 dlclose -> SEGFAULT（random timing）
	if (api_handle != NULL)
	{
		dlclose(api_handle);
		api_handle = NULL;
	}
	printf("Press any keyword to terminate the process...\n");
	getchar();

	exit(0);
}

bool export_api()
{
// Export the APIs
	fp_msg_dumper_get_version = (FP_msg_dumper_get_version)dlsym(api_handle, "msg_dumper_get_version");
	if (fp_msg_dumper_get_version == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_version() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_set_severity_index = (FP_msg_dumper_set_severity_index)dlsym(api_handle, "msg_dumper_set_severity_index");
	if (fp_msg_dumper_set_severity_index == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_severity_index() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_set_severity = (FP_msg_dumper_set_severity)dlsym(api_handle, "msg_dumper_set_severity");
	if (fp_msg_dumper_set_severity == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_severity() due to %s\n", dlerror());
		return false;
	}
	// fp_msg_dumper_set_facility = (FP_msg_dumper_set_facility)dlsym(api_handle, "msg_dumper_set_facility");
	// if (fp_msg_dumper_set_facility == NULL)
	// {
	// 	fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_facility() due to %s\n", dlerror());
	// 	return false;
	// }
	fp_msg_dumper_get_severity_index = (FP_msg_dumper_get_severity_index)dlsym(api_handle, "msg_dumper_get_severity_index");
	if (fp_msg_dumper_get_severity_index == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_severity() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_get_severity = (FP_msg_dumper_get_severity)dlsym(api_handle, "msg_dumper_get_severity");
	if (fp_msg_dumper_get_severity == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_severity() due to %s\n", dlerror());
		return false;
	}
	// fp_msg_dumper_get_facility = (FP_msg_dumper_get_facility)dlsym(api_handle, "msg_dumper_get_facility");
	// if (fp_msg_dumper_get_facility == NULL)
	// {
	// 	fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_facility() due to %s\n", dlerror());
	// 	return false;
	// }
	fp_msg_dumper_initialize = (FP_msg_dumper_initialize)dlsym(api_handle, "msg_dumper_initialize");
	if (fp_msg_dumper_initialize == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_initialize() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_write_msg = (FP_msg_dumper_write_msg)dlsym(api_handle, "msg_dumper_write_msg");
	if (fp_msg_dumper_write_msg == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_write_msg() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_write_format_msg = (FP_msg_dumper_write_format_msg)dlsym(api_handle, "msg_dumper_write_format_msg");
	if (fp_msg_dumper_write_format_msg == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_write_format_msg() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_deinitialize = (FP_msg_dumper_deinitialize)dlsym(api_handle, "msg_dumper_deinitialize");
	if (fp_msg_dumper_deinitialize == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_deinitialize() due to %s\n", dlerror());
		return false;
	}
	fp_msg_dumper_get_error_description = (FP_msg_dumper_get_error_description)dlsym(api_handle, "msg_dumper_get_error_description");
	if (fp_msg_dumper_get_error_description == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_error_description() due to %s\n", dlerror());
		return false;
	}
	return true;
}

unsigned short parse_config()
{
	static const int BUF_SIZE = 256;
	char current_working_directory[BUF_SIZE];
	getcwd(current_working_directory, sizeof(current_working_directory));
	// char config_filename[BUF_SIZE];
	// snprintf(config_filename, BUF_SIZE, "%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
	ScopedCStr scoped_config_filename;
	// safe_snprintf(scoped_config_filename.out(), BUF_SIZE, "%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
	scoped_config_filename.format("%s/%s/%s", current_working_directory, CONF_FOLDER, CONF_FILENAME);
	#ifdef DO_DEBUG
	printf("Parse the config file: %s\n", scoped_config_filename.get());
#endif
	FILE *fp = fopen(scoped_config_filename.get(), "r");
	if (fp == NULL)
	{
		fprintf(stderr, "fopen() fails, reason: %s\n", strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	// unsigned short total_facility_flag = 0x0;
	unsigned short ret = MSG_DUMPER_SUCCESS;
// Parse the config file
	char buf[BUF_SIZE];
	while (fgets(buf, BUF_SIZE, fp) != NULL)
	{
		if (buf[0] == '[')
			break;
		if (buf[0] == '\n' || buf[0] == '#')
			continue;
		int split_pos = -1;
		char* sch = strchr(buf, '=');
		if (sch == NULL)
		{
			fprintf(stderr, "Incorrect config format: %s\n", buf);
			return MSG_DUMPER_FAILURE_INCORRECT_CONFIG;
		}
		split_pos = sch - buf;
		char* ech = strchr(buf, '\n');
		if (ech != NULL)
		{
			int end_pos = ech - buf;
			if (buf[end_pos - 1] == '\r')
				end_pos--;
			buf[end_pos] = '\0';
		}
		string config(buf);
//		WRITE_DEBUG_FORMAT_LOGGING(STRING_SIZE, "***Config*** content: %s, split pos: %d", config.c_str(), split_pos);
		char facility_name[16];
		snprintf(facility_name, 16, "%s", config.substr(0, split_pos).c_str());
		char severity_name[16];
		snprintf(severity_name, 16, "%s", config.substr(split_pos + 1).c_str());
#ifdef DO_DEBUG
		fprintf(stderr, "***Config*** facility: %s, severity: %s\n", facility_name, severity_name);
#endif
// Set facility and severity
// 		unsigned short facility_index = get_facility_index_from_string(facility_name);
// 		total_facility_flag |= FACILITY_FLAG[facility_index];
// // Set severity
// 		unsigned short linux_severity = get_linux_severity_from_string(severity_name);
// #ifdef DO_DEBUG
// 		fprintf(stderr, "Set severity of facility[%s] to %s\n", FACILITY_NAME[facility_index], SEVERITY_NAME[get_msg_dumper_severity_from_string(severity_name)]);
// #endif
// 		ret = fp_msg_dumper_set_severity(linux_severity, FACILITY_FLAG[facility_index]);
		ret = fp_msg_dumper_set_severity(facility_name, severity_name);
		if (CHECK_FAILURE(ret))
		{
			fprintf(stderr, "fp_msg_dumper_set_severity() fails, reason: %s\n", fp_msg_dumper_get_error_description());
			goto OUT;
		}
	}
// #ifdef DO_DEBUG
// 	printf("Set facility to %d\n", total_facility_flag);
// #endif
// 	ret = fp_msg_dumper_set_facility(total_facility_flag);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_set_facility() fails, reason: %s\n", fp_msg_dumper_get_error_description());
// 		return ret;
// 	}
OUT:
	fclose(fp);
	fp = NULL;

	return ret;
}

// unsigned short MsgDumperWrapper::get_facility_index_from_string(const char* facility_name)
// {
// 	assert(facility_name != NULL && "facility_name should NOT be NULL");
// 	unsigned short facility;
// 	bool found = false;
// 	for (int i = 0 ; i < FACILITY_NAME_SIZE ; i++)
// 	{
// #ifdef DO_DEBUG
// 		fprintf(stderr, "%s, %s, %d\n", facility_name, FACILITY_NAME[i], strcmp(facility_name, FACILITY_NAME[i]));
// #endif
// 		if (strcmp(facility_name, FACILITY_NAME[i]) == 0)
// 		{
// 			facility = i;
// 			found = true;
// 			break;
// 		}
// 	}
// 	if (!found)
// 	{
// 		char exception[64];
// 		snprintf(exception, 64, "Unknown facility: %s", facility_name);
// 		throw invalid_argument(string(exception));
// 	}
// 	return facility;
// }

// bool test()
// {
// 	bool test_pass = false;
// 	int severity_size;
// 	const char** severity_name_list = MsgDumperWrapper::get_severity_name_list(severity_size);
// 	printf("Severity Name: ");
// 	for (int i = 0 ; i < severity_size ; i++)
// 		printf("%s ", severity_name_list[i]);
// 	printf("\n");
// 	int facility_size;
// 	const char** facility_name_list = MsgDumperWrapper::get_facility_name_list(facility_size);
// 	printf("Facility Name: ");
// 	for (int i = 0 ; i < facility_size ; i++)
// 		printf("%s ", facility_name_list[i]);
// 	printf("\n");

// 	MsgDumperWrapper* msg_dumper = MsgDumperWrapper::get_instance();
// 	unsigned short ret = MSG_DUMPER_SUCCESS;
// 	unsigned short old_log_severity;
// 	unsigned short new_log_severity;

// 	printf("Get Log severity config: %d\n", msg_dumper->get_log_severity_config());
// 	printf("Get Syslog severity config: %d\n", msg_dumper->get_syslog_severity_config());

// 	printf("Get Log severity: %d, %s\n", msg_dumper->get_log_severity(), msg_dumper->get_log_severity_by_name());
// 	printf("Get Syslog severity: %d, %s\n", msg_dumper->get_syslog_severity(), msg_dumper->get_syslog_severity_by_name());

// // Write the message
// 	printf("Write the ERROR message\n");
// 	ret = msg_dumper->write(__FILE__, __LINE__, LOG_ERR, "This is a test: Error");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_ERR, "This is a format test: %s, %d", "Error", LOG_ERR);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write the WARN message\n");
// 	ret = msg_dumper->write(__FILE__, __LINE__, LOG_WARNING, "This is a test: Warn");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_WARNING, "This is a format test: %s, %d", "Warn", LOG_WARNING);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write the INFO message\n");
// 	ret = msg_dumper->write(__FILE__, __LINE__, LOG_INFO, "This is a test: Info");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_INFO, "This is a format test: %s, %d", "Info", LOG_INFO);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write the DEBUG message\n");
// 	ret = msg_dumper->write(__FILE__, __LINE__, LOG_DEBUG, "This is a test: Debug");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_DEBUG, "This is a format test: %s, %d", "Debug", LOG_DEBUG);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// // Try different serverity
// 	old_log_severity = msg_dumper->get_log_severity();
// 	new_log_severity = LOG_WARNING;
// 	msg_dumper->set_log_severity(new_log_severity);
// 	printf("Switch Log severity from [%d] to [%d]\n", old_log_severity, msg_dumper->get_log_severity());

// // Write the message after switch log severity
// 	printf("Write another ERROR message\n");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_ERR, "This is another format test: %s, %d", "Error", LOG_ERR);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write another WARN message\n");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_WARNING, "This is another format test: %s, %d", "Warn", LOG_WARNING);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write another INFO message\n");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_INFO, "This is another format test: %s, %d", "Info", LOG_INFO);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	sleep(1);
// // Write the message
// 	printf("Write another DEBUG message\n");
// 	ret = msg_dumper->format_write(__FILE__, __LINE__, LOG_DEBUG, "This is another format test: %s, %d", "Debug", LOG_DEBUG);
// 	if (CHECK_FAILURE(ret))
// 	{
// 		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper->get_error_description());
// 		goto EXIT1;
// 	}
// 	msg_dumper->set_log_severity_config(2);
// 	msg_dumper->set_syslog_severity_config(1);
// 	printf("Get Log severity: %d\n", msg_dumper->get_log_severity_config());
// 	printf("Get Syslog Severity: %d\n", msg_dumper->get_syslog_severity_config());
// 	test_pass = true;
// EXIT1:
// 	SAFE_RELEASE(msg_dumper);
// 	return test_pass;
// }

// bool test_macro()
// {
// 	bool test_pass = false;
// 	DECLARE_MSG_DUMPER()
// 	IMPLEMENT_MSG_DUMPER()
// 	// unsigned short ret = MSG_DUMPER_SUCCESS;
// // Write the message
// 	printf("Write the ERROR message through Macro\n");
// 	WRITE_ERROR("This is a test through Macro: Error");
// 	WRITE_FORMAT_ERROR("This is a format test through Macro: %s, %d", "Error", LOG_ERR);
// 	sleep(1);
// // Write the message
// 	printf("Write the WARN message through Macro\n");
// 	WRITE_WARN("This is a test through Macro: Warn");
// 	WRITE_FORMAT_WARN("This is a format test through Macro: %s, %d", "Warn", LOG_WARNING);
// 	sleep(1);
// // Write the message
// 	printf("Write the INFO message through Macro\n");
// 	WRITE_INFO("This is a test through Macro: Info");
// 	WRITE_FORMAT_INFO("This is a format test through Macro: %s, %d", "Info", LOG_INFO);
// 	sleep(1);
// // Write the message
// 	printf("Write the DEBUG message through Macro\n");
// 	WRITE_DEBUG("This is a test through Macro: Debug");
// 	WRITE_FORMAT_DEBUG("This is a format test through Macro: %s, %d", "Debug", LOG_DEBUG);
// 	test_pass = true;
// // EXIT1:
// 	RELEASE_MSG_DUMPER()
// 	return test_pass;
// }