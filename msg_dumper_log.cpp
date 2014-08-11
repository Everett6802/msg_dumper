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
	log_filename(NULL),
	log_filepath(NULL)
{

}

MsgDumperLog::~MsgDumperLog()
{
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
//	deinitialize();
}

unsigned short MsgDumperLog::create_device_file()
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
	log_filename = new char[MSG_DUMPER_SHORT_STRING_SIZE];
	log_filepath = new char[MSG_DUMPER_SHORT_STRING_SIZE];
	if (log_filename == NULL || log_filepath == NULL)
	{
		WRITE_DEBUG_SYSLOG("Fail to allocate the memory: log_filename/log_filepath");
		return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
	}
	memset(log_filename, 0x0, sizeof(char) * MSG_DUMPER_SHORT_STRING_SIZE);
	snprintf(log_filename, MSG_DUMPER_SHORT_STRING_SIZE, "%02d%02d%02d%02d%02d.log", (p->tm_year + 1900) % 2000, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min);
	memset(log_filepath, 0x0, sizeof(char) * MSG_DUMPER_SHORT_STRING_SIZE);
	snprintf(log_filepath, MSG_DUMPER_SHORT_STRING_SIZE, "%s/%s", LOG_FOLDER, log_filename);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log file path: %s", log_filepath);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_device_file()
{
// Open the file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the log file: %s", log_filename);
	FILE* fp = fopen(log_filepath, "a");
	if (fp == NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to open log file: %s", log_filename);
		return MSG_DUMPER_FAILURE_OPEN_FILE;
	}

// Write the message into the log file
	for (int i = 0 ; i < write_vector.size() ; i++)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to file [%s]", write_vector[i], log_filename);
		fputs(write_vector[i], fp);
// Release the resource
		delete[] write_vector[i];
		write_vector[i] = NULL;
	}

// Clean-up the container
	write_vector.clear();

// Close the file
	if (fp != NULL)
		fclose(fp);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::create_log_folder()
{
	struct stat st = {0};

	char folder_path[MSG_DUMPER_STRING_SIZE];
	snprintf(folder_path, MSG_DUMPER_SHORT_STRING_SIZE, "./%s", LOG_FOLDER);

// Check if the log folder exists or not
	if (stat(folder_path, &st) == -1)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to create a log folder: %s", folder_path);
// If not, create a new folder
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
// Create the log folder
	unsigned short ret = create_log_folder();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
	return ret;

	return MsgDumperTimerThread::initialize(config);
}
