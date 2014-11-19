#include <fcntl.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_sql.h"

/*
 * Go to the following links to see more detailed info:
 * http://www.cs.wichita.edu/~chang/lecture/cs742/program/how-mysql-c-api.html
 * http://docstore.mik.ua/orelly/linux/sql/ch19_01.htm
 */

char* MsgDumperSql::server = "localhost";
char* MsgDumperSql::username = "root";
char* MsgDumperSql::password = "lab4man1";
char* MsgDumperSql::database = "msg_dumper";
char* MsgDumperSql::format_cmd_create_database = "CREATE DATABASE %s";
char* MsgDumperSql::format_cmd_create_table = "CREATE TABLE sql%s (date VARCHAR(16), time VARCHAR(16), data VARCHAR(512))";

MsgDumperSql::MsgDumperSql() :
	connection(NULL)
{

}

MsgDumperSql::~MsgDumperSql()
{
	WRITE_DEBUG_SYSLOG("The destructor of MsgDumperSql is called...");
}

unsigned short MsgDumperSql::try_connect_mysql()
{
	WRITE_DEBUG_SYSLOG("Try to connect to the MySQL database server...");
	connection = mysql_init(NULL); // 初始化数据库连接变量
	if(connection == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_init() fails, due to: %s", mysql_error(connection));
		return MSG_DUMPER_FAILURE_MYSQL;
	}

// 函数mysql_real_connect建立一个数据库连接，成功返回MYSQL*连接句柄，失败返回NULL
	if(mysql_real_connect(connection, server, username, password, database, 0, NULL, 0) == NULL)
	{
// The database does NOT exist !!! Try to create one
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The %s database does NOT exist, create a NEW one", database);
// mysql_create_db() has been deprecated in the newer releases of MySQL. MySQL now supports the CREATE DATABASE SQL statement.
// This should be used, via the mysql_query function, instead
//		if(mysql_create_db(connection, database) != 0)
		if(mysql_real_connect(connection, server, username, password, NULL, 0, NULL, 0) == NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_real_connect() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}

		snprintf(cmd_buf, FORMAT_CMD_BUF_SIZE, format_cmd_create_database, database);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to create database[%s] by command: %s", database, cmd_buf);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_query() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The %s database is created", database);
	}
	WRITE_DEBUG_SYSLOG("Try to connect to the MySQL database server...... Successfully");

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::create_device_file()
{
	unsigned short ret = try_connect_mysql();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;
// Get the current time
	char current_time_string[CURRENT_TIME_STRING_LENGTH];
	generate_current_time_string(current_time_string);

// Create the table in the database...
	snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_create_table, current_time_string);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to create table[sql%s] by command: %s", current_time_string, cmd_buf);
	if(mysql_query(connection, cmd_buf) != NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_query() fails, due to: %s", mysql_error(connection));
		return MSG_DUMPER_FAILURE_MYSQL;
	}

	if (connection != NULL)
	{
		mysql_close(connection);
		connection = NULL;
	}

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

// Create the specific table
	unsigned short ret = create_device_file();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

	device_handle_exist = true;

	return MsgDumperTimerThread::initialize(config);
}

unsigned short MsgDumperSql::deinitialize()
{
	WRITE_DEBUG_SYSLOG("DeInitialize the MsgDumperSql object......");

	WRITE_DEBUG_SYSLOG("Close the connection to the MySQL database");
	if(connection != NULL)  // 关闭数据库连接
	{
		mysql_close(connection);
		connection = NULL;
	}

	return MsgDumperTimerThread::deinitialize();
}
