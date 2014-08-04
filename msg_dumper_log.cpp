#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "msg_dumper_log.h"
#include "msg_dumper.h"
#include "common.h"


char* MsgDumperLog::LOG_FOLDER = "log";

MsgDumperLog::MsgDumperLog() :
	exit(0),
	log_filename(NULL),
	log_filepath(NULL)
{

}

void* MsgDumperLog::msg_dumper_log_handler(void* void_ptr)
{
	if (void_ptr != NULL)
	{
		MsgDumperLog* this_ptr = (MsgDumperLog*)void_ptr;
		if (CHECK_MSG_DUMPER_FAILURE(this_ptr->msg_dumper_log_handler_internal()))
		{
			pthread_exit((void*)"Failure");
		}
	}

	pthread_exit((void*)"Success");
}

unsigned short MsgDumperLog::msg_dumper_log_handler_internal()
{
	WRITE_DEBUG_SYSLOG("The worker thread of writing message is running");
	while (!exit)
	{
		WRITE_DEBUG_SYSLOG("Run!");
		sleep(1);
	}

	WRITE_DEBUG_SYSLOG("The worker thread of writing message is going to die");
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::create_logfile()
{
	if (log_filename != NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log file[%s] has already been created", log_filename);
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
// Create the log file
	time_t timep;
	time(&timep);
	struct tm* p = localtime(&timep);
	log_filename = new char[32];
	log_filepath = new char[32];
	if (log_filename == NULL || log_filepath == NULL)
	{
		WRITE_DEBUG_SYSLOG("Fail to allocate the memory: log_filename/log_filepath");
		return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
	}
	memset(log_filename, 0x0, sizeof(char) * 32);
	snprintf(log_filename, 32, "%02d%02d%02d%02d%02d.log", (p->tm_year + 1900) % 2000, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min);
	memset(log_filepath, 0x0, sizeof(char) * 32);
	snprintf(log_filepath, 32, "%s/%s", LOG_FOLDER, log_filename);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log file path: %s", log_filepath);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_logfile()
{
// Open the file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the log file: %s", log_filename);
	FILE* fp = fopen(log_filepath, "a");
	if (fp == NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to open log file: %s", log_filename);
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}

	fclose(fp);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::create_logfolder()
{
	struct stat st = {0};

	char folder_path[MSG_DUMPER_STRING_SIZE];
	snprintf(folder_path, MSG_DUMPER_SHORT_STRING_SIZE, "./%s", LOG_FOLDER);

	if (stat(folder_path, &st) == -1)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to create a log folder: %s", folder_path);
	    if (mkdir(folder_path, 0744) != 0)
	    {
	    	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to create a log folder[%s], due to %s", folder_path, strerror(errno));
	    	return MSG_DUMPER_FAILURE_UNKNOWN;
	    }
	}
	else
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log folder[%s] has already existed", folder_path);
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::initialize(void* config)
{
	int api_ret;
// Create the workder thread
	api_ret = pthread_create(&pid, NULL, msg_dumper_log_handler, this);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_create() failed, due to: %d", api_ret);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
// Create the log folder
	unsigned short ret = create_logfolder();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_msg(const char* msg)
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	if (log_filename == NULL)
	{
// Create the log file if it does not exist
		ret = create_logfile();
		if (CHECK_MSG_DUMPER_FAILURE(ret))
			return ret;
	}

// Get the time that the message is generated
	time_t timep;
	time(&timep);
	struct tm *p = localtime(&timep);
	char time_string[MSG_DUMPER_SHORT_STRING_SIZE];
	snprintf(time_string, MSG_DUMPER_SHORT_STRING_SIZE, "%02d/%02d/%02d %02d:%02d:%02d", (1900 + p->tm_year) % 2000, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

// Add the message into the list
	char *new_msg = new char[MSG_DUMPER_LONG_STRING_SIZE];
	if (new_msg == NULL)
	{
		WRITE_ERR_SYSLOG("Fail to allocate the memory: new_msg");
		return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
	}
	memset(new_msg, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
	snprintf(new_msg, MSG_DUMPER_LONG_STRING_SIZE, "[%s] %s\n", time_string, msg);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "New message: %s", new_msg);

	FILE* fp = fopen(log_filepath, "a");
	if (fp == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_SHORT_STRING_SIZE, "Fail to open the file: %s", log_filepath);
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}
	fputs(new_msg, fp);
	if (fp != NULL)
		fclose(fp);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	WRITE_DEBUG_SYSLOG("Try to kill the worker thread...");
	__sync_lock_test_and_set(&exit, 1);

	int api_ret;
	char api_ret_desc[256];
	api_ret = pthread_join(pid, (void**)&api_ret_desc);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_join() failed, due to: %s", api_ret_desc);
		return MSG_DUMPER_FAILURE_UNKNOWN;
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

	return MSG_DUMPER_SUCCESS;
}
