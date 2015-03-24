#ifndef MSG_DUMPER_REMOTE_H
#define MSG_DUMPER_REMOTE_H

#include <stdio.h>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "msg_dumper_base.h"
#include "msg_dumper_mgr.h"


using namespace std;

class MsgDumperRemote : public MsgDumperBase
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
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperRemote();
	virtual ~MsgDumperRemote();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* config_path, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
