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
	pid(0),
	exit(0),
	log_filename(NULL),
	log_filepath(NULL)
{

}

MsgDumperLog::~MsgDumperLog()
{
//	deinitialize();
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
	unsigned short ret = MSG_DUMPER_SUCCESS;
	WRITE_DEBUG_SYSLOG("The worker thread of writing message is running");
	while (!exit)
	{
		pthread_mutex_lock (&mut);
//wait for the signal with cond as condition variable
		pthread_cond_wait(&cond, &mut);

// Move the message
		int buffer_vector_size = buffer_vector.size();
		if (buffer_vector_size > 0)
		{
			for (int i = 0 ; i < buffer_vector_size ; i++)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Move the message[%s] to another buffer", buffer_vector[i]);
				char* elem =  buffer_vector[i];
				buffer_vector[i] = NULL;
				write_vector.push_back(elem);
			}
// Clean-up the container
			buffer_vector.clear();
		}
		pthread_mutex_unlock (&mut);

		if (write_vector.size() > 0)
		{
			ret = write_logfile();
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				break;
		}
	}

	WRITE_DEBUG_SYSLOG("The worker thread of writing message is going to die");
	return ret;
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

unsigned short MsgDumperLog::create_logfolder()
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
	int api_ret;
// Create the worker thread
	api_ret = pthread_create(&pid, NULL, msg_dumper_log_handler, this);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_create() failed, due to: %d", api_ret);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}

	exit = 0;
// Initialize the synchronization object
	mut = PTHREAD_MUTEX_INITIALIZER;
	cond = PTHREAD_COND_INITIALIZER;

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

	pthread_mutex_lock(&mut);
	buffer_vector.push_back(new_msg);
// Wake up waiting thread with condition variable, if it is called before this function
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mut);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	if (exit == 0)
	{
		WRITE_DEBUG_SYSLOG("Try to kill the worker thread...");
		__sync_lock_test_and_set(&exit, 1);

// Notify the worker thread to exit
		pthread_mutex_lock(&mut);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mut);

		int api_ret;
		char api_ret_desc[256];
		api_ret = pthread_join(pid, (void**)&api_ret_desc);
		if (api_ret != 0)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_join() failed, due to: %s", api_ret_desc);
			return MSG_DUMPER_FAILURE_UNKNOWN;
		}
		pid = 0;
	}

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mut);

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
