#include <fcntl.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_com.h"


char* MsgDumperCom::COM_PORT_NAME = "/dev/ttyUSB1";
speed_t MsgDumperCom::COM_PORT_SPEED = B115200;

MsgDumperCom::MsgDumperCom()
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "COM");
}

MsgDumperCom::~MsgDumperCom()
{
}

unsigned short MsgDumperCom::try_open_comport(int& fd_com)const
{
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the COM port file: %s", COM_PORT_NAME);
	int fd_comport = open(COM_PORT_NAME, O_RDWR | O_NOCTTY);

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
	cfsetospeed (&tty, COM_PORT_SPEED);
	cfsetispeed (&tty, COM_PORT_SPEED);
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
	return MSG_DUMPER_SUCCESS;
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
