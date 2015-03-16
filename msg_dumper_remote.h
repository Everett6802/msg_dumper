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


private:
	static char* DEF_SERVER_PORT;

	class RemoteServerCfg;
	typedef RemoteServerCfg* PREMOTESERVERCFG;

	char server_list[MSG_DUMPER_STRING_SIZE];
	char server_port[MSG_DUMPER_STRING_SIZE];
	vector<PREMOTESERVERCFG> server_socket_vector;

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
