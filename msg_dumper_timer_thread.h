#ifndef MSG_DUMPER_TIMER_THREAD_H
#define MSG_DUMPER_TIMER_THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


#define CURRENT_TIME_STRING_LENGTH 11

using namespace std;

class MsgDumperTimerThread : public MsgDumperBase
{
	friend class MsgDumperMgr;

	class MsgCfg
	{
	public:
		time_t timep;
		struct tm *p;
		char date_str[MSG_DUMPER_SHORT_STRING_SIZE];
		char time_str[MSG_DUMPER_SHORT_STRING_SIZE];
		unsigned short severity;
		char data[MSG_DUMPER_LONG_STRING_SIZE];

		MsgCfg(const time_t& new_timep, unsigned short new_severity, const char* new_data)
		{
			timep = new_timep;
			p = localtime(&timep);
			snprintf(date_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d/%02d/%02d", (1900 + p->tm_year) % 2000, p->tm_mon + 1, p->tm_mday);
			snprintf(time_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);

			severity = new_severity;
			memset(data, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
			snprintf(data, MSG_DUMPER_LONG_STRING_SIZE, "%s", new_data);
		}

		unsigned short create_format_message(char* format_message, int format_message_len)
		{
// Add the message into the list
			if (format_message == NULL)
			{
				WRITE_ERR_SYSLOG("Invalid argument: format_message");
				return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			}

			memset(format_message, 0x0, sizeof(char) * format_message_len);
			snprintf(format_message, format_message_len, "[%s %s %s] %s\n", date_str, time_str, MSG_DUMPER_SEVERITY_DESC[severity], data);

			return MSG_DUMPER_SUCCESS;
		}
	};
	typedef MsgCfg* PMSG_CFG;

private:
	pthread_t pid;
	int exit;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	bool thread_is_running;

	static void* msg_dumper_thread_handler(void* void_ptr);
	unsigned short msg_dumper_thread_handler_internal();

protected:
	vector<PMSG_CFG> buffer_vector;
	vector<PMSG_CFG> write_vector;
	char format_message[MSG_DUMPER_LONG_STRING_SIZE];

	MsgDumperTimerThread();
	virtual ~MsgDumperTimerThread();

	unsigned short generate_current_time_string(char* current_time_string);

	virtual unsigned short create_device_file()=0;
	virtual unsigned short write_device_file()=0;

public:
	virtual unsigned short initialize(void* config=NULL);
	virtual unsigned short deinitialize();

	unsigned short write_msg(const time_t& timep, unsigned short severity, const char* msg);
};

#endif
