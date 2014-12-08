#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../msg_dumper.h"
#include "../common.h"


unsigned short daemon_init();

int main()
{
	unsigned short ret = MSG_DUMPER_SUCCESS;

	WRITE_DEBUG_SYSLOG("Start the MsgDumper daemon...");
	ret = daemon_init();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		exit(EXIT_SUCCESS);

	int server_sockfd;
// Initialize the server socket
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd == -1)
	{
		WRITE_ERR_SYSLOG("Fail to create a server socket: server_sockfd");
		exit(EXIT_FAILURE);
	}

// Bind
	int server_len;
	struct sockaddr_in server_address;
	memset(&server_address, 0x0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = SERVER_PORT_NO;
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr*)&server_address, server_len);

// Listen
	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	listen(server_sockfd, MAX_CONNECTED_CLIENT);

	while (true)
	{
		WRITE_INFO_SYSLOG("Server waits for client's request......");
// Accept the request from the client
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, (socklen_t*)&client_len);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Got a connection request from %s......", inet_ntoa(client_address.sin_addr));

//// Read the command from the client
//		unsigned int doit_packet_data_header;
//		read(client_sockfd, (void*)&doit_packet_data_header, sizeof(unsigned int));
//		WRITE_DEBUG_FORMAT_SYSLOG(DOIT_STRING_SIZE, "The packet header value: %d", doit_packet_data_header);
//
//// Trigger the thread to start the task
//		WRITE_DEBUG_SYSLOG("Trigger the thread to start the task......");
//		try
//		{
//			if (CHECK_FAILURE(thread_manager.take_action(doit_action, doit_action_type, doit_action_param)))
//			WRITE_ERR_FORMAT_SYSLOG(DOIT_STRING_SIZE, "Error while taking action[%d], action type[%d]", doit_action, doit_action_type);
//		}
//		catch(...)
//		{
//			WRITE_ERR_SYSLOG("Error occur while trying to start the action...");
//		}
//		close(client_sockfd);
	}

	exit(EXIT_SUCCESS);
}

unsigned short daemon_init()
{
// Step 1: Fork off the parent process
	int childpid = fork();
	if(childpid == -1)
	{
		WRITE_ERR_SYSLOG("fork() error");
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
	else if(childpid > 0)
		exit(EXIT_SUCCESS);
// Step 2: Create a unique session ID
	if(setsid()== -1)
	{
		WRITE_ERR_SYSLOG("setsid() error");
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
// Step 3: Change the working directory
	chdir("/");
// Step 4: Close the standard file descriptors
	/*
	The getdtablesize() system call returns the maximum number of file descriptors that the cur-
	rent process may open. The maximum file descriptor number that the system may assign is the
	return value minus one.
	*/
	int fdtablesize = getdtablesize();
	for(int fd = 0 ; fd < fdtablesize ; fd++)
		close(fd);
// Step 5: Change the file mode mask
	umask(0);
	return MSG_DUMPER_SUCCESS;
}
