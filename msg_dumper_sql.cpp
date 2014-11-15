#include <fcntl.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_sql.h"


MsgDumperSql::MsgDumperSql()
{

}

MsgDumperSql::~MsgDumperSql()
{
}

unsigned short MsgDumperSql::try_open_mysql(int& fd_com)const
{
//	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the COM port file: %s", COM_PORT_NAME);
//	int fd_comport = open(COM_PORT_NAME, O_RDWR | O_NOCTTY);


	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::create_device_file()
{
//	int fd_com;
//	unsigned short ret = try_open_comport(fd_com);
//	if (CHECK_MSG_DUMPER_FAILURE(ret))
//		return ret;
//	close(fd_com);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::write_device_file()
{
//	int fd_com;
//// Open the COM port
//	unsigned short ret = try_open_comport(fd_com);
//	if (CHECK_MSG_DUMPER_FAILURE(ret))
//		return ret;
//
//// Write the message into the log file
//	for (int i = 0 ; i < write_vector.size() ; i++)
//	{
//		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to COM port", write_vector[i]);
//		write(fd_com, write_vector[i], strlen(write_vector[i]));
//// Release the resource
//		delete[] write_vector[i];
//		write_vector[i] = NULL;
//	}
//// Clean-up the container
//	write_vector.clear();
//
//// Close the COM port
//	close(fd_com);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::initialize(void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperSql object......");
//// Create the log folder
//	unsigned short ret = create_device_file();
//	if (CHECK_MSG_DUMPER_FAILURE(ret))
//		return ret;
//	device_handle_exist = true;

	return MsgDumperTimerThread::initialize(config);
}
