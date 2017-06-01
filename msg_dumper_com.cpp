#include <fcntl.h>
#include "msg_dumper.h"
#include "msg_dumper_com.h"


const char* MsgDumperCom::DEF_COM_PORT_NAME = "/dev/ttyUSB1";
const char* MsgDumperCom::DEF_COM_PORT_SPEED = "B115200";

MsgDumperCom::MsgDumperCom() :
	fd_com(0)
{
	memcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_COM], strlen(MSG_DUMPER_FACILITY_DESC[FACILITY_COM]));
	memset(port_name, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(port_name, DEF_COM_PORT_NAME, sizeof(char) * strlen(DEF_COM_PORT_NAME));
	memset(port_speed, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(port_speed, DEF_COM_PORT_SPEED, sizeof(char) * strlen(DEF_COM_PORT_SPEED));
	transform_com_port_speed(com_port_speed);
	memset(&tty_new, 0x0, sizeof(tty_new));
	memset(&tty_original, 0x0, sizeof(tty_original));
}

MsgDumperCom::~MsgDumperCom()
{
}

unsigned short MsgDumperCom::try_open_comport()
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the COM port file: %s", port_name);
	int fd_comport = open(port_name, O_RDWR | O_NOCTTY);

/* Error Handling */
	if (tcgetattr(fd_comport, &tty_new) != 0)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to get COM port attribute, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}

/* Save old tty parameters */
	tty_original = tty_new;
/* Set Baud Rate */
	unsigned short ret = transform_com_port_speed(com_port_speed);
	if (CHECK_FAILURE(ret))
		return ret;

	cfsetospeed (&tty_new, com_port_speed);
	cfsetispeed (&tty_new, com_port_speed);
/* Setting other Port Stuff */
	tty_new.c_cflag &= ~PARENB;        // Make 8n1
	tty_new.c_cflag &= ~CSTOPB;
	tty_new.c_cflag &= ~CSIZE;
	tty_new.c_cflag |= CS8;
	tty_new.c_cflag &= ~CRTSCTS;       // no flow control
	tty_new.c_cc[VMIN] = 1;            // read doesn't block
	tty_new.c_cc[VTIME] = 5;           // 0.5 seconds read timeout
	tty_new.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines
/* Make raw */
	cfmakeraw(&tty_new);
/* Flush Port, then applies attributes */
	tcflush(fd_comport, TCIFLUSH);
	if(tcsetattr(fd_comport, TCSANOW, &tty_new) != 0)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to set COM port attribute, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}
	close(fd_comport);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::transform_com_port_speed(speed_t& com_port_speed)const
{
	static const char* support_com_port_speed_string[] = {"9600", "115200"};
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

unsigned short MsgDumperCom::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static const char* title[] = {"port_name", "port_speed"};
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

unsigned short MsgDumperCom::open_device()
{
// Open the COM port
	fd_com = open(port_name, O_RDWR | O_NOCTTY);
	if (fd_com == -1)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to open COM port, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}

// Set the configuration
	if(tcsetattr(fd_com, TCSANOW, &tty_new) != 0)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to set COM port attribute, due to %s", strerror(errno));
		return MSG_DUMPER_FAILURE_COM_PORT;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::close_device()
{
// Close the COM port
	if (fd_com != 0)
	{
		close(fd_com);
		fd_com = 0;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::initialize(const char* current_working_directory, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperCom object......");

// Parse the config file first
	unsigned short ret = parse_config(current_working_directory, "com");
	if (CHECK_FAILURE(ret))
		return ret;

// Try to connect to the serial port
	ret = try_open_comport();
	if (CHECK_FAILURE(ret))
		return ret;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::deinitialize()
{
	WRITE_DEBUG_SYSLOG("DeInitialize the MsgDumperCom object......");

	return close_device();
}

unsigned short MsgDumperCom::write_msg(PMSG_CFG msg_cfg)
{
	if (fd_com == 0)
	{
		WRITE_ERR_SYSLOG("The file handle does NOT connect to serial port");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

// Write the message into the log file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to COM port", msg_cfg->to_string());
	const char* msg_cfg_string = msg_cfg->to_string();
	int acutal_len = write(fd_com, msg_cfg_string, strlen(msg_cfg_string));
	if (acutal_len == -1)
	{
		WRITE_ERR_SYSLOG("Fail to write the data to the serial port...");
		return MSG_DUMPER_FAILURE_UNKNOWN ;
	}

	return MSG_DUMPER_SUCCESS;
}
