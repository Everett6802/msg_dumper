#ifndef MSG_DUMPER_REMOTE_H
#define MSG_DUMPER_REMOTE_H

#include <stdio.h>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


using namespace std;

class MsgDumperRemote : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;

	class RemoteServerCfg
	{
		static const int IP_LEN = 16;

	public:
		char ip[IP_LEN];
		int sockfd;

		RemoteServerCfg(const char* new_ip) :
			sockfd(-1)
		{
			memset(ip, 0x0, sizeof(char) * IP_LEN);
			memcpy(ip, new_ip, strlen(new_ip));
		}

		~RemoteServerCfg()
		{
			if (sockfd != 0)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Close the connection: %s", ip);
				close(sockfd);
				sockfd = -1;
			}
		}
	};
	typedef RemoteServerCfg* PREMOTESERVERCFG;

private:
	static char* DEF_SERVER_PORT;

	char server_list[MSG_DUMPER_STRING_SIZE];
	char server_port[MSG_DUMPER_STRING_SIZE];
	list<PREMOTESERVERCFG> server_socket_list;

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperRemote();
	virtual ~MsgDumperRemote();

	virtual unsigned short initialize(const char* config_path, void* config=NULL);
	virtual unsigned short deinitialize();
};

#endif
