#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "msg_dumper.h"
#include "msg_dumper_wrapper.h"


#define SAFE_RELEASE(x) x.release();
#define CHECK_FAILURE(x) ((x != MSG_DUMPER_SUCCESS) ? true : false)

using namespace std;

int main()
{
	MsgDumperWrapper& msg_dumper = *MsgDumperWrapper::get_instance();
	unsigned short ret = MSG_DUMPER_SUCCESS;

// Write the message
	printf("Write the ERROR message\n");
	// ret = msg_dumper.write(LOG_ERR, "This is a test: Error");
	// ret = msg_dumper.format_write(LOG_ERR, "This is a format test: %s, %d", "Error", LOG_ERR);
	ret = msg_dumper.error("This is a test: Error");
	ret = msg_dumper.format_error("This is a format test: %s, %d", "Error", LOG_ERR);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(3);
// Write the message
	printf("Write the WARN message\n");
	// ret = msg_dumper.write(LOG_WARNING, "This is a test: Warn");
	// ret = msg_dumper.format_write(LOG_WARNING, "This is a format test: %s, %d", "Warn", LOG_WARNING);
	ret = msg_dumper.warn("This is a test: Warn");
	ret = msg_dumper.format_warn("This is a format test: %s, %d", "Warn", LOG_WARNING);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(3);
// Write the message
	printf("Write the INFO message\n");
	// ret = msg_dumper.write(LOG_INFO, "This is a test: Info");
	// ret = msg_dumper.format_write(LOG_INFO, "This is a format test: %s, %d", "Info", LOG_INFO);
	ret = msg_dumper.info("This is a test: Info");
	ret = msg_dumper.format_info("This is a format test: %s, %d", "Info", LOG_INFO);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(3);
// Write the message
	printf("Write the DEBUG message\n");
	// ret = msg_dumper.write(LOG_DEBUG, "This is a test: Debug");
	// ret = msg_dumper.format_write(LOG_DEBUG, "This is a format test: %s, %d", "Debug", LOG_DEBUG);
	ret = msg_dumper.debug("This is a test: Debug");
	ret = msg_dumper.format_debug("This is a format test: %s, %d", "Debug", LOG_DEBUG);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	// msg_dumper.set_log_severity_config(2);
	// msg_dumper.set_syslog_severity_config(1);
	// printf("Get Log severity: %d\n", msg_dumper.get_log_severity_config());
	// printf("Get Syslog Severity: %d\n", msg_dumper.get_syslog_severity_config());

	printf("Press any keyword to terminate the process...\n");
	getchar();

EXIT1:
	SAFE_RELEASE(msg_dumper)

	exit(EXIT_SUCCESS);
}
