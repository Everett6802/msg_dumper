#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "msg_dumper_timer_thread.h"


MsgDumperTimerThread::MsgDumperTimerThread() :
	pid(0),
	exit(0),
	thread_is_running(false),
	new_data_trigger(false)
{
}

MsgDumperTimerThread::~MsgDumperTimerThread()
{
//	deinitialize();
}

void* MsgDumperTimerThread::msg_dumper_thread_handler(void* void_ptr)
{
	if (void_ptr != NULL)
	{
		MsgDumperTimerThread* this_ptr = (MsgDumperTimerThread*)void_ptr;
		if (CHECK_MSG_DUMPER_FAILURE(this_ptr->msg_dumper_thread_handler_internal()))
		{
			pthread_exit((void*)"Failure");
		}
	}

	pthread_exit((void*)"Success");
}

unsigned short MsgDumperTimerThread::msg_dumper_thread_handler_internal()
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The worker thread is running", get_thread_name());
	while (!exit)
	{
		pthread_mutex_lock(&mut);
//wait for the signal with cond as condition variable
		if (!new_data_trigger)
			pthread_cond_wait(&cond, &mut);
//		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The worker thread to write the data......", get_thread_name());

// Move the message
		int buffer_vector_size = buffer_vector.size();
//		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Thread[%s]=> There are totally %d data in the queue", get_thread_name(), buffer_vector_size);
		if (buffer_vector_size > 0)
		{
			for (int i = 0 ; i < buffer_vector_size ; i++)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Thread[%s]=> Move the message[%s] to another buffer", get_thread_name(), buffer_vector[i]->data);
				PMSG_CFG elem =  buffer_vector[i];
				buffer_vector[i] = NULL;
				write_vector.push_back(elem);
			}
// Clean-up the container
			buffer_vector.clear();
		}
		new_data_trigger = 0;
		pthread_mutex_unlock(&mut);

		if (write_vector.size() > 0)
		{
			ret = write_device_file();
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				break;
		}
	}

	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The worker thread of writing message is going to die", get_thread_name());
	return ret;
}

unsigned short MsgDumperTimerThread::initialize(const char* config_path, void* config)
{
	int api_ret;
// Initialize the synchronization object
	mut = PTHREAD_MUTEX_INITIALIZER;
	cond = PTHREAD_COND_INITIALIZER;

	WRITE_DEBUG_SYSLOG("Create a worker thread......");
// Create the worker thread
	api_ret = pthread_create(&pid, NULL, msg_dumper_thread_handler, this);
	if (api_ret != 0)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_create() failed, due to: %d", api_ret);
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
	else
	{
		thread_is_running = true;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperTimerThread::write_msg(const time_t& timep, unsigned short severity, const char* msg)
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	if (!device_handle_exist)
	{
// Create the device file if it does not exist
		ret = create_device_file();
		if (CHECK_MSG_DUMPER_FAILURE(ret))
			return ret;
		device_handle_exist = true;
	}

	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [severity: %d, message: %s]", severity, msg);
	PMSG_CFG new_msg = new MsgCfg(timep, severity, msg);

	pthread_mutex_lock(&mut);
	buffer_vector.push_back(new_msg);
// Wake up waiting thread with condition variable, if it is called before this function
	if (!new_data_trigger)
		pthread_cond_signal(&cond);
	new_data_trigger = true;
	pthread_mutex_unlock(&mut);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperTimerThread::deinitialize()
{
	// Before killing this thread, check if the thread is STILL alive
	if (thread_is_running)
	{
// You should make sure the thread(pid) is created correctly before executing the pthread_kill() function. Otherwise, there are some errors dumped from the kernel
		int kill_ret = pthread_kill(pid, 0);
		if(kill_ret == ESRCH)
			WRITE_ERR_SYSLOG("The worker thread in the MsgDumperTimerThread object did NOT exist......");
		else if(kill_ret == EINVAL)
			WRITE_ERR_SYSLOG("The signal in the MsgDumperTimerThread object is invalid");
		else
		{
			WRITE_DEBUG_SYSLOG("The worker thread in the MsgDumperTimerThread object is STILL alive");
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
			WRITE_DEBUG_SYSLOG("The worker thread in the MsgDumperTimerThread object is dead");
		}
		thread_is_running = false;
	}

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mut);

    device_handle_exist = false;

// Check if there are some messages left in the Queue
    if (buffer_vector.size() > 0)
    {
    	WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "#####   ERROR buffer vector is NOT empty: %d   #####", buffer_vector.size());
    }
    if (write_vector.size() > 0)
    {
    	WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "#####   ERROR write vector is NOT empty: %d   #####", write_vector.size());
    }

	return MSG_DUMPER_SUCCESS;
}

const char* MsgDumperTimerThread::get_thread_name()const
{
	return worker_thread_name;
}

