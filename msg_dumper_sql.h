#ifndef MSG_DUMPER_SQL_H
#define MSG_DUMPER_SQL_H

#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperSql : public MsgDumperBase
{
private:
	static const char* DEF_SERVER;
	static const char* DEF_USERNAME;
	static const char* DEF_PASSWORD;
	static const char* DEF_DATABASE;

	static const char* format_cmd_create_database;
	static const char* format_cmd_create_table;
	static const char* format_cmd_insert_into_table;

	MYSQL* connection;
	char cmd_buf[MSG_DUMPER_LONG_STRING_SIZE];
	char current_time_string[CURRENT_TIME_STRING_LENGTH];
	bool table_created;
	char server[MSG_DUMPER_STRING_SIZE];
	char username[MSG_DUMPER_STRING_SIZE];
	char password[MSG_DUMPER_STRING_SIZE];
	char database[MSG_DUMPER_STRING_SIZE];

	unsigned short try_connect_mysql();

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperSql();
	virtual ~MsgDumperSql();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* current_working_directory, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
