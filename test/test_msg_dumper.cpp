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
FP_msg_dumper_set_facility fp_msg_dumper_set_facility;
FP_msg_dumper_write_msg fp_msg_dumper_write_msg;
FP_msg_dumper_deinitialize fp_msg_dumper_deinitialize;

bool export_api(void* handle);



using namespace std;

//void test_fmt_string(char* fmt_string, int fmt_string_len, const char* fmt, ...)
//{
//	if (fmt_string == NULL)
//	{
//
//	}
//
//	static const int fmtbuf_len = 16;
//	static char fmtbuf[fmtbuf_len];
//
//	va_list arg_list;
//	va_start(arg_list, fmt);
//
//	char* arg_str = NULL;
//	int arg_str_len = 0;
//	int fmt_string_index = 0;
//	bool out = false;
//	for(const char* p = fmt; *p != '\0' && fmt_string_index < fmt_string_len ; p++)
//	{
//		if (*p == '%')
//		{
//			switch (*++p)
//			{
//				case 'd':
//				{
//	// itoa is not ansi C standard and you should probably avoid it. Here are some roll-your-own implementations if you really want to use it anyway:
//	//				fputs(itoa(value, fmtbuf, 10), stdout);
//					snprintf(fmtbuf, fmtbuf_len, "%d", va_arg(arg_list, int));
//					arg_str = fmtbuf;
//				}
//				break;
//				case 's':
//				{
//					arg_str = va_arg(arg_list, char*);
//				}
//				break;
//				default:
//				{
//					assert(0 && "Unsupported format");
//				}
//				break;
//			}
//			arg_str_len = strlen(arg_str);
//			if (fmt_string_index + arg_str_len >= fmt_string_len - 1)
//			{
//				arg_str_len = fmt_string_len - fmt_string_index - 1;
//				out = true;
//			}
//			memcpy(&fmt_string[fmt_string_index], arg_str, sizeof(char) * arg_str_len);
//			fmt_string_index += arg_str_len;
//	//		fputs(arg_str, stdout);
//
//		}
//		else
//		{
////			putchar(*p);
//			fmt_string[fmt_string_index++] = *p;
//		}
//	}
//	fmt_string[fmt_string_index] = '\0';
//	va_end(arg_list);
//}

int main()
{
//	char buf[16];
//	test_fmt_string(buf, 16, "%d %d %d", 1, 2, 3);
//	printf("%s\n", buf);
//	test_fmt_string(buf, 16, "%s %s %s", "Fuck", "Your", "Mother");
//	printf("%s\n", buf);
//	exit(0);

// Load library
	unsigned short ret = MSG_DUMPER_SUCCESS;
	void* handle = NULL;
	handle = dlopen("libmsg_dumper.so", RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "dlopen() fails, due to %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	unsigned short severity = MSG_DUMPER_SEVIRITY_DEBUG;
	unsigned short facility = MSG_DUMPER_FACILITY_SYSLOG;

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
	printf("Set severity to :%d\n", severity);
	ret = fp_msg_dumper_set_severity(severity);
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_set_severity() fails, due to %d\n", ret);
		goto EXIT;
	}

// Set facility
	printf("Set facility to :%d\n", facility);
	ret = fp_msg_dumper_set_facility(facility);
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_set_facility() fails, due to %d\n", ret);
		goto EXIT;
	}

// Initialize the library
	printf("Initialize the library\n");
	ret = fp_msg_dumper_initialize("..");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_initialize() fails, due to %d\n", ret);
		goto EXIT1;
	}

	sleep(3);

// Write the message
	printf("Write the ERROR message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_ERROR, "This is a test: Error");
//	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_ERROR, "This is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: ErrorThis is a test: Error");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the WARN message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_WARN, "This is a test: Warn");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the INFO message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_INFO, "This is a test: Info");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d\n", ret);
		goto EXIT1;
	}

// Write the message
	printf("Write the DEBUG message\n");
	ret = fp_msg_dumper_write_msg(MSG_DUMPER_SEVIRITY_DEBUG, "This is a test: Debug");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
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
