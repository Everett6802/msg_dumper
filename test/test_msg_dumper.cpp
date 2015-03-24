#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "../msg_dumper.h"


FP_msg_dumper_initialize fp_msg_dumper_initialize;
FP_msg_dumper_get_version fp_msg_dumper_get_version;
FP_msg_dumper_set_severity fp_msg_dumper_set_severity;
FP_msg_dumper_set_severity_all fp_msg_dumper_set_severity_all;
FP_msg_dumper_set_facility fp_msg_dumper_set_facility;
FP_msg_dumper_get_severity fp_msg_dumper_get_severity;
FP_msg_dumper_get_facility fp_msg_dumper_get_facility;
FP_msg_dumper_write_msg fp_msg_dumper_write_msg;
FP_msg_dumper_write_format_msg fp_msg_dumper_write_format_msg;
FP_msg_dumper_deinitialize fp_msg_dumper_deinitialize;

bool export_api(void* handle);
int get_device_type_amount();

using namespace std;

int main()
{
// Load library
	unsigned short ret = MSG_DUMPER_SUCCESS;
	void* handle = NULL;
	handle = dlopen("libmsg_dumper.so", RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "dlopen() fails, due to %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	char* facility_name[] = {"Log", "Com", "Sql", "Remote", "Syslog"};
//	unsigned short severity_arr[] = {MSG_DUMPER_SEVIRITY_DEBUG};
//	unsigned short facility = MSG_DUMPER_FACILITY_LOG;
	unsigned short severity_arr[] = {MSG_DUMPER_SEVIRITY_ERROR, MSG_DUMPER_SEVIRITY_WARN, MSG_DUMPER_SEVIRITY_DEBUG};
	unsigned short facility = MSG_DUMPER_FACILITY_LOG | MSG_DUMPER_FACILITY_SQL | MSG_DUMPER_FACILITY_SYSLOG;

	int device_type_amount = get_device_type_amount();
	int flags = 0x1;

// Export the APIs
	if (!export_api(handle))
	{
		fprintf(stderr, "Fail to export the APIs\n");
		goto EXIT;
	}

	unsigned char major_version;
	unsigned char minor_version;
	fp_msg_dumper_get_version(major_version, minor_version);
	printf("API version: (%d.%d)\n", major_version, minor_version);

// Set severity
	for (int i = 0, severity_cnt = 0 ; i < device_type_amount ; i++)
	{
		if (flags & facility)
		{
			printf("Set severity of facility[%s] to %d\n", facility_name[i], severity_arr[severity_cnt]);
			ret = fp_msg_dumper_set_severity(severity_arr[severity_cnt], flags);
			if (CHECK_FAILURE(ret))
			{
				fprintf(stderr, "fp_msg_dumper_set_severity() fails, due to %d\n", ret);
				goto EXIT;
			}
			severity_cnt++;
		}
		flags <<= 1;
	}

// Set facility
	printf("Set facility to %d\n", facility);
	ret = fp_msg_dumper_set_facility(facility);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_set_facility() fails, due to %d\n", ret);
		goto EXIT;
	}

// Initialize the library
	printf("Initialize the library\n");
	ret = fp_msg_dumper_initialize();
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_initialize() fails, due to %d\n", ret);
		goto EXIT1;
	}

	sleep(3);

// Write the message
	printf("Write the ERROR message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_ERROR, "This is a test: Error");
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVIRITY_ERROR, "This is a format test: %s, %d", "Error", MSG_DUMPER_SEVIRITY_ERROR);
//	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_ERROR, "This is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: Error");
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the WARN message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_WARN, "This is a test: Warn");
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVIRITY_WARN, "This is a format test: %s, %d", "Warn", MSG_DUMPER_SEVIRITY_WARN);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the INFO message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_INFO, "This is a test: Info");
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVIRITY_INFO, "This is a format test: %s, %d", "Info", MSG_DUMPER_SEVIRITY_INFO);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the DEBUG message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_DEBUG, "This is a test: Debug");
	ret = fp_msg_dumper_write_format_msg(MSG_DUMPER_SEVIRITY_DEBUG, "This is a format test: %s, %d", "Debug", MSG_DUMPER_SEVIRITY_DEBUG);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

	printf("Press any keyword to terminate the process...\n");
	getchar();

EXIT1:
// De-initialize the library
	printf("Close the library\n");
	fp_msg_dumper_deinitialize();

EXIT:
// Close the handle
	if (handle != NULL)
	{
		dlclose(handle);
		handle = NULL;
	}

	exit(EXIT_SUCCESS);
}

int get_device_type_amount()
{
	unsigned short flag = 0x1;
	int count = 0;
	while (MSG_DUMPER_FACILITY_ALL & flag)
	{
		count++;
		flag <<= 1;
	}

	return count;
}

bool export_api(void* handle)
{
	fp_msg_dumper_get_version = (FP_msg_dumper_get_version)dlsym(handle, "msg_dumper_get_version");
	if (fp_msg_dumper_get_version == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_version() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_initialize = (FP_msg_dumper_initialize)dlsym(handle, "msg_dumper_initialize");
	if (fp_msg_dumper_initialize == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_initialize() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_set_severity = (FP_msg_dumper_set_severity)dlsym(handle, "msg_dumper_set_severity");
	if (fp_msg_dumper_set_severity == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_severity() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_set_severity_all = (FP_msg_dumper_set_severity_all)dlsym(handle, "msg_dumper_set_severity_all");
	if (fp_msg_dumper_set_severity_all == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_severity_all() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_set_facility = (FP_msg_dumper_set_facility)dlsym(handle, "msg_dumper_set_facility");
	if (fp_msg_dumper_set_facility == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_facility() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_get_severity = (FP_msg_dumper_get_severity)dlsym(handle, "msg_dumper_get_severity");
	if (fp_msg_dumper_get_severity == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_severity() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_get_facility = (FP_msg_dumper_get_facility)dlsym(handle, "msg_dumper_get_facility");
	if (fp_msg_dumper_get_facility == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_facility() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_write_msg = (FP_msg_dumper_write_msg)dlsym(handle, "msg_dumper_write_msg");
	if (fp_msg_dumper_write_msg == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_write_msg() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_write_format_msg = (FP_msg_dumper_write_format_msg)dlsym(handle, "msg_dumper_write_format_msg");
	if (fp_msg_dumper_write_format_msg == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_write_format_msg() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_deinitialize = (FP_msg_dumper_deinitialize)dlsym(handle, "msg_dumper_deinitialize");
	if (fp_msg_dumper_deinitialize == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_deinitialize() due to %s\n", dlerror());
		return false;
	}

	return true;
}
