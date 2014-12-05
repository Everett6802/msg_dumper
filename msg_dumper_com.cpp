#include <fcntl.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_com.h"


char* MsgDumperCom::DEF_COM_PORT_NAME = "/dev/ttyUSB1";
char* MsgDumperCom::DEF_COM_PORT_SPEED = "B115200";

MsgDumperCom::MsgDumperCom()
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "COM");
	memcpy(port_name, DEF_COM_PORT_NAME, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(port_speed, DEF_COM_PORT_SPEED, sizeof(char) * MSG_DUMPER_STRING_SIZE);
}

MsgDumperCom::~MsgDumperCom()
{
}

unsigned short MsgDumperCom::try_open_comport(int& fd_com)const
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the COM port file: %s", port_name);
	int fd_comport = open(port_name, O_RDWR | O_NOCTTY);

	struct termios tty;
	struct termios tty_old;
	memset(&tty, 0x0, sizeof(tty));

/* Error Handling */
	if (tcgetattr(fd_comport, &tty) != 0)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to get COM port attribute, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}

/* Save old tty parameters */
	tty_old = tty;
/* Set Baud Rate */
	speed_t com_port_speed;
	unsigned short ret = transform_com_port_speed(com_port_speed);
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	cfsetospeed (&tty, com_port_speed);
	cfsetispeed (&tty, com_port_speed);
/* Setting other Port Stuff */
	tty.c_cflag &= ~PARENB;        // Make 8n1
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;       // no flow control
	tty.c_cc[VMIN] = 1;            // read doesn't block
	tty.c_cc[VTIME] = 5;           // 0.5 seconds read timeout
	tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines
/* Make raw */
	cfmakeraw(&tty);
/* Flush Port, then applies attributes */
	tcflush(fd_comport, TCIFLUSH);
	if(tcsetattr(fd_comport, TCSANOW, &tty) != 0)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to set COM port attribute, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}
	fd_com = fd_comport;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::transform_com_port_speed(speed_t& com_port_speed)const
{
	static char* support_com_port_speed_string[] = {"9600", "115200"};
	static speed_t support_com_port_speed[] = {B9600, B115200};
	static const int support_com_port_speed_len = sizeof support_com_port_speed / sizeof support_com_port_speed[0];

	for (int index = 0 ; index < support_com_port_speed_len ; index++)
	{
		if (strcmp(port_speed, support_com_port_speed_string[index]) == 0)
		{
			com_port_speed = support_com_port_speed[index];
			return MSG_DUMPER_SUCCESS;
		}
	}

	WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Un-support COM port speed: %s", port_speed);
	return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
}

unsigned short MsgDumperCom::create_device_file()
{
	int fd_com;
	unsigned short ret = try_open_comport(fd_com);
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;
	close(fd_com);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::write_device_file()
{
	int fd_com;
// Open the COM port
	unsigned short ret = try_open_comport(fd_com);
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

// Write the message into the log file
	for (int i = 0 ; i < write_vector.size() ; i++)
	{
		PMSG_CFG msg_cfg = write_vector[i];
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to COM port", msg_cfg->to_string());
		write(fd_com, msg_cfg->to_string(), strlen(msg_cfg->to_string()));
// Release the resource
		delete[] msg_cfg;
		write_vector[i] = NULL;
	}
// Clean-up the container
	write_vector.clear();

// Close the COM port
	close(fd_com);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static char* title[] = {"port_name", "port_speed"};
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
				param_member_variable = port_name;
				break;
			case 1:
				param_member_variable = port_speed;
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

unsigned short MsgDumperCom::initialize(void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperCom object......");

// Parse the config file first
	unsigned short ret = parse_config("com");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

// Create the log folder
	ret = create_device_file();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;
	device_handle_exist = true;

	return MsgDumperTimerThread::initialize(config);
}
