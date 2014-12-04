#ifndef MSG_DUMPER_TIMER_THREAD_H
#define MSG_DUMPER_TIMER_THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


#define CURRENT_TIME_STRING_LENGTH 11

using namespace std;

class MsgDumperTimerThread : public MsgDumperBase
{
	friend class MsgDumperMgr;

protected:
	class MsgCfg
	{
	private:
		time_t timep;
		struct tm *p;
		char* format_message;

	public:
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

			format_message = NULL;
		}

//		~MsgCfg()
//		{
//			if (format_message != NULL)
//			{
//				delete[] format_message;
//				format_message = NULL;
//			}
//		}

		const char* to_string()
		{
			if (format_message == NULL)
			{
				format_message = new char[MSG_DUMPER_LONG_STRING_SIZE];
				if (format_message == NULL)
				{
					assert(0 && "Fail to allocate the format_message");
					return NULL;
				}
				snprintf(format_message, MSG_DUMPER_LONG_STRING_SIZE, "[%s %s %s] %s\n", date_str, time_str, MSG_DUMPER_SEVERITY_DESC[severity], data);
			}
			return format_message;
		}
	};
	typedef MsgCfg* PMSG_CFG;

private:
	static const char* CONF_FOLDER_NAME;
	pthread_t pid;
	int exit;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	bool thread_is_running;
	bool new_data_trigger;

	static void* msg_dumper_thread_handler(void* void_ptr);
	unsigned short msg_dumper_thread_handler_internal();

protected:
	vector<PMSG_CFG> buffer_vector;
	vector<PMSG_CFG> write_vector;
	char worker_thread_name[MSG_DUMPER_SHORT_STRING_SIZE];

	MsgDumperTimerThread();
	virtual ~MsgDumperTimerThread();

	pthread_t get_pid()const{return pid;}
	unsigned short generate_current_time_string(char* current_time_string);
	unsigned short parse_config(const char* dev_name);

	virtual unsigned short create_device_file()=0;
	virtual unsigned short write_device_file()=0;
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content)=0;

	virtual const char* get_thread_name()const;

public:
	virtual unsigned short initialize(void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(const time_t& timep, unsigned short severity, const char* msg);
};

#endif
