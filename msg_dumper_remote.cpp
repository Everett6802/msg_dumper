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
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to connect to the server: %s...... Successfully", pRemoteServerCfg->ip);

		server_socket_vector.push_back(pRemoteServerCfg);

		tok = strtok(NULL, ",");
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperRemote::write_device_file()
{
	static string end_of_packet = "\r\n\r\n";
//	static int end_of_packet_len = end_of_packet.size();

// Write the message into the log file
	int write_vector_size = write_vector.size();
	int server_socket_vector_size = server_socket_vector.size();
	for (int i = 0 ; i < write_vector_size ; i++)
	{
		PMSG_CFG msg_cfg = (PMSG_CFG)write_vector[i];
		for(int j = 0 ; j < server_socket_vector_size ; j++)
		{
			PREMOTESERVERCFG remote_server_cfg = (PREMOTESERVERCFG)server_socket_vector[i];
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to remote server [%s]", msg_cfg->to_string(), remote_server_cfg->ip);

			string packet_data = string(msg_cfg->to_string()) + end_of_packet;
			int msg_cfg_len = packet_data.size(); //strlen(msg_cfg->to_string());
			int numbytes = write(remote_server_cfg->sockfd, packet_data.c_str()/*msg_cfg->to_string()*/, msg_cfg_len);
			if (numbytes == -1)
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Fail to write the message[%s] to remote server [%s]", msg_cfg->to_string(), remote_server_cfg->ip);
				return MSG_DUMPER_FAILURE_SOCKET;
			}
			else if (numbytes < msg_cfg_len)
			{
				int index_pointer = numbytes;
				int left_msg_cfg_len = msg_cfg_len - numbytes;
				const char* msg_cfg_content = packet_data.c_str(); //msg_cfg->to_string();
				while (left_msg_cfg_len > 0)
				{
					numbytes = write(remote_server_cfg->sockfd, &msg_cfg_content[index_pointer], left_msg_cfg_len);
					if (numbytes == -1)
					{
						WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Fail to write the message[%s] to remote server [%s]", msg_cfg->to_string(), remote_server_cfg->ip);
						return MSG_DUMPER_FAILURE_SOCKET;
					}
					index_pointer += numbytes;
					left_msg_cfg_len -= numbytes;
				}
			}
		}

// Release the resource
		delete[] msg_cfg;
		write_vector[i] = NULL;
	}

// Clean-up the container
	write_vector.clear();

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

unsigned short MsgDumperRemote::initialize(const char* config_path, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperRemote object......");

// Parse the config file first
	unsigned short ret = parse_config(config_path, "remote");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	return MsgDumperTimerThread::initialize(config_path, config);
}

unsigned short MsgDumperRemote::deinitialize()
{
	int server_socket_vector_size = server_socket_vector.size();
	for(int i = 0 ; i < server_socket_vector_size ; i++)
	{
		PREMOTESERVERCFG pRemoteServerCfg = (PREMOTESERVERCFG)server_socket_vector[i];
		server_socket_vector[i] = NULL;
		delete pRemoteServerCfg;
	}
	server_socket_vector.clear();

	return MsgDumperTimerThread::deinitialize();
}
