#ifndef MSG_DUMPER_SQL_H
#define MSG_DUMPER_SQL_H

#include <mysql/mysql.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


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
	static char* format_cmd_insert_into_table;

	MYSQL* connection;
	char cmd_buf[MSG_DUMPER_LONG_STRING_SIZE];
	char current_time_string[CURRENT_TIME_STRING_LENGTH];
	bool table_created;

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
