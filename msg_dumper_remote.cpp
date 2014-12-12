#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "msg_dumper_remote.h"


char* MsgDumperRemote::DEF_SERVER_PORT = "6802";

MsgDumperRemote::MsgDumperRemote()
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "REMOTE");
	memset(server_list, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memset(server_port, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
}

MsgDumperRemote::~MsgDumperRemote()
{

//	deinitialize();
}

unsigned short MsgDumperRemote::create_device_file()
{
	char* tok = strtok(server_list, ",");
	int server_port_no = atoi(server_port);
	while(tok)
	{
		PREMOTESERVERCFG pRemoteServerCfg = new RemoteServerCfg(tok);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to connect to the server: %s......", tok);

// Initialize the client socket
		pRemoteServerCfg->sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (pRemoteServerCfg->sockfd == -1)
		{
			WRITE_ERR_SYSLOG("Fail to create a client socket: client_sockfd");
			return MSG_DUMPER_FAILURE_SOCKET;
		}

// Connect
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Client connect to server[%s]", pRemoteServerCfg->ip);
		int client_len;
		struct sockaddr_in client_address;
		memset(&client_address, 0x0, sizeof(struct sockaddr_in));
		client_address.sin_family = AF_INET;
		client_address.sin_port = htons(server_port_no);
		client_address.sin_addr.s_addr = inet_addr(pRemoteServerCfg->ip);
		if (connect(pRemoteServerCfg->sockfd, (struct sockaddr*)&client_address, sizeof(struct sockaddr)) == -1)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to connect to server[%s]", pRemoteServerCfg->ip);
			return MSG_DUMPER_FAILURE_SOCKET;
		}

		server_socket_list.push_back(pRemoteServerCfg);

		tok = strtok(NULL, ",");
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::write_device_file()
{

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static char* title[] = {"server_list", "server_port"};
	static int title_len = sizeof title / sizeof title[0];

	unsigned short ret = MSG_DUMPER_SUCCESS;
	bool found = false;
	for (int index = 0 ; index < title_len ; index++)
	{
		if (strcmp(title[index], param_title) == 0)
		{
			int param_content_len = strlen(param_content);
			char* param_member_variable = NULL;
			switch(index)
			{
			case 0:
				param_member_variable = server_list;
				break;
			case 1:
				param_member_variable = server_port;
				break;
			}

			if (param_member_variable != NULL)
			{
				memset(param_member_variable, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
				memcpy(param_member_variable, param_content, param_content_len);
				found = true;
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Update parameter: %s=%s", param_title, param_content);
			}
			else
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter: %s=%s", param_title, param_content);
				ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
			}
			break;
		}
	}
// If the title is NOT found...
	if (!found)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter, fail to find the title: %s", param_title);
		ret = MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	return ret;

}

unsigned short MsgDumperRemote::initialize(void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperRemote object......");

// Parse the config file first
	unsigned short ret = parse_config("remote");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	return MsgDumperTimerThread::initialize(config);
}

unsigned short MsgDumperRemote::deinitialize()
{
	list<PREMOTESERVERCFG>::iterator iter = server_socket_list.begin();
	while(iter++ != server_socket_list.end())
	{
		PREMOTESERVERCFG pRemoteServerCfg = (PREMOTESERVERCFG)*iter;
		*iter = NULL;
		delete pRemoteServerCfg;
	}
	server_socket_list.clear();

	return MsgDumperTimerThread::deinitialize();
}
