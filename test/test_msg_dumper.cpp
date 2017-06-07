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

	int severity_size;
	const char** severity_name_list = msg_dumper.get_severity_name_list(severity_size);
	printf("Severity Name: ");
	for (int i = 0 ; i < severity_size ; i++)
		printf("%s ", severity_name_list[i]);
	printf("\n");
	int facility_size;
	const char** facility_name_list = msg_dumper.get_facility_name_list(facility_size);
	printf("Facility Name: ");
	for (int i = 0 ; i < facility_size ; i++)
		printf("%s ", facility_name_list[i]);
	printf("\n");

	unsigned short ret = MSG_DUMPER_SUCCESS;
	unsigned short old_log_severity;
	unsigned short new_log_severity;

	printf("Get Log severity config: %d\n", msg_dumper.get_log_severity_config());
	printf("Get Syslog severity config: %d\n", msg_dumper.get_syslog_severity_config());

	printf("Get Log severity: %d, %s\n", msg_dumper.get_log_severity(), msg_dumper.get_log_severity_by_name());
	printf("Get Syslog severity: %d, %s\n", msg_dumper.get_syslog_severity(), msg_dumper.get_syslog_severity_by_name());

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
	sleep(1);
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
	sleep(1);
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
	sleep(1);
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
// Try different serverity
	old_log_severity = msg_dumper.get_log_severity();
	new_log_severity = LOG_WARNING;
	msg_dumper.set_log_severity(new_log_severity);
	printf("Switch Log severity from [%d] to [%d]\n", old_log_severity, msg_dumper.get_log_severity());

// Write the message after switch log severity
	printf("Write another ERROR message\n");
	ret = msg_dumper.format_error("This is another format test: %s, %d", "Error", LOG_ERR);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(1);
// Write the message
	printf("Write another WARN message\n");
	ret = msg_dumper.format_warn("This is another format test: %s, %d", "Warn", LOG_WARNING);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(1);
// Write the message
	printf("Write another INFO message\n");
	ret = msg_dumper.format_info("This is another format test: %s, %d", "Info", LOG_INFO);
	if (CHECK_FAILURE(ret))
	{
		fprintf(stderr, "fp_msg_dumper_write_msg() fails, due to %d, reason: %s\n", ret, msg_dumper.get_error_description());
		goto EXIT1;
	}
	sleep(1);
// Write the message
	printf("Write another DEBUG message\n");
	ret = msg_dumper.format_debug("This is another format test: %s, %d", "Debug", LOG_DEBUG);
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
