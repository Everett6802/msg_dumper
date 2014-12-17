#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <vector>
#include "../msg_dumper.h"
#include "../common.h"
#include "msg_dumper_daemon.h"


using namespace std;

unsigned short daemon_init();
void* read_socket_thread_handler(void* void_param);

int main()
{
	unsigned short ret = MSG_DUMPER_SUCCESS;
	vector<pthread_t> worker_thread_vector;

	WRITE_DEBUG_SYSLOG("Start the MsgDumper daemon...");
	ret = daemon_init();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		exit(EXIT_SUCCESS);

	int server_sockfd;
// Initialize the server socket
	WRITE_DEBUG_SYSLOG("Initialize the server socket in MsgDumper daemon...");
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd == -1)
	{
		WRITE_ERR_SYSLOG("socket() fail");
		exit(EXIT_FAILURE);
	}

// Bind
	int server_len;
	struct sockaddr_in server_address;
	WRITE_DEBUG_SYSLOG("Bind the server socket in MsgDumper daemon...");
	memset(&server_address, 0x0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT_NO);
	server_len = sizeof(server_address);
	if (bind(server_sockfd, (struct sockaddr*)&server_address, server_len) == -1)
	{
		WRITE_ERR_SYSLOG("bind() fail");
		exit(EXIT_FAILURE);
	}

// Listen
	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	WRITE_DEBUG_SYSLOG("The server socket is listening in MsgDumper daemon...");
	if (listen(server_sockfd, MAX_CONNECTED_CLIENT) == -1)
	{
		WRITE_ERR_SYSLOG("listen() fail");
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		WRITE_INFO_SYSLOG("Server waits for client's request......");
// Accept the request from the client
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, (socklen_t*)&client_len);
		if (client_sockfd == -1)
		{
			WRITE_ERR_SYSLOG("accept() fail");
			exit(EXIT_FAILURE);
		}
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Got a connection request from the remote[%s]......", inet_ntoa(client_address.sin_addr));

// Create a worker thread to receive the data from the remote site
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Create another thread to receive the data from the remote[%s]", inet_ntoa(client_address.sin_addr));
		pthread_t pid;
		int res = pthread_create(&pid, NULL, read_socket_thread_handler, (void*)&client_sockfd);
		if (res != 0)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "pthread_create() fail, due to %s", strerror(res));
			exit(EXIT_FAILURE);
		}
	}
	close(client_sockfd);

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

void* read_socket_thread_handler(void* void_param)
{
	static const int BUF_SIZE = 256;
	int client_socket = *((int*)void_param);
	int numbytes;
	char buf[BUF_SIZE];

	WRITE_DEBUG_SYSLOG("The worker thread of receiving data daemon is running......");

	while (true)
	{
		numbytes = read(client_socket, buf, sizeof(char) * BUF_SIZE);
		if (numbytes == -1)
			pthread_exit((void*)"Fail to read the data");
		WRITE_DEBUG_SYSLOG(buf);
	}

	pthread_exit((void*)"Success");
}
