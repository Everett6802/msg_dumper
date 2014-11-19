#ifndef MSG_DUMPER_SQL_H
#define MSG_DUMPER_SQL_H

#include <mysql/mysql.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


#define FORMAT_CMD_BUF_SIZE 64

class MsgDumperSql : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	static char* server;
	static char* username;
	static char* password;
	static char* database;

	static char* format_cmd_create_database;
	static char* format_cmd_create_table;

	MYSQL* connection;
	char cmd_buf[FORMAT_CMD_BUF_SIZE];
	unsigned short try_connect_mysql();

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();

public:
	MsgDumperSql();
	virtual ~MsgDumperSql();

	virtual unsigned short initialize(void* config=NULL);
	virtual unsigned short deinitialize();
};

#endif
