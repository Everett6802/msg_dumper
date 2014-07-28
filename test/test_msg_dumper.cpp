#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "../msg_dumper.h"


FP_msg_dumper_initialize fp_msg_dumper_initialize;
FP_msg_dumper_get_version fp_msg_dumper_get_version;
FP_msg_dumper_set_severity fp_msg_dumper_set_severity;
FP_msg_dumper_set_facility fp_msg_dumper_set_facility;
FP_msg_dumper_write_msg fp_msg_dumper_write_msg;
FP_msg_dumper_deinitialize fp_msg_dumper_deinitialize;


bool export_api(void* handle);

int main()
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	void* handle = NULL;
	handle = dlopen("libmsg_dumper.so", RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "dlopen() fails, due to %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	if (!export_api(handle))
	{
		fprintf(stderr, "Fail to export the APIs\n");
		goto EXIT;
	}

//	unsigned char major_version;
//	unsigned char minor_version;
//	fp_msg_dumper_get_version(major_version, minor_version);
//	printf("API version: %d, %d\n", major_version, minor_version);

	ret = fp_msg_dumper_initialize();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_initialize() fails, due to %d\n", ret);
		goto EXIT1;
	}

EXIT1:
	fp_msg_dumper_deinitialize();

EXIT:
	if (handle != NULL)
	{
		dlclose(handle);
		handle = NULL;
	}

	exit(EXIT_SUCCESS);
}

bool export_api(void* handle)
{
	fp_msg_dumper_initialize = (FP_msg_dumper_initialize)dlsym(handle, "msg_dumper_initialize");
	if (fp_msg_dumper_initialize == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_initialize() due to %s\n", dlerror());
		return false;
	}

//	fp_msg_dumper_get_version = (FP_msg_dumper_get_version)dlsym(handle, "msg_dumper_get_version");
//	if (fp_msg_dumper_get_version == NULL)
//	{
//		fprintf(stderr, "dlsym() fails when exporting msg_dumper_get_version() due to %s\n", dlerror());
//		return false;
//	}

	fp_msg_dumper_set_severity = (FP_msg_dumper_set_severity)dlsym(handle, "msg_dumper_set_severity");
	if (fp_msg_dumper_set_severity == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_severity() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_set_facility = (FP_msg_dumper_set_facility)dlsym(handle, "msg_dumper_set_facility");
	if (fp_msg_dumper_set_facility == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_set_facility() due to %s\n", dlerror());
		return false;
	}

	fp_msg_dumper_write_msg = (FP_msg_dumper_write_msg)dlsym(handle, "msg_dumper_write_msg");
	if (fp_msg_dumper_write_msg == NULL)
	{
		fprintf(stderr, "dlsym() fails when exporting msg_dumper_write_msg() due to %s\n", dlerror());
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
