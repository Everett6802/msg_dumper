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
char* MsgDumperSql::format_cmd_create_table = "CREATE TABLE sql%s (date VARCHAR(16), time VARCHAR(16), severity INT, data VARCHAR(512))";
char* MsgDumperSql::format_cmd_insert_into_table = "INSERT INTO sql%s VALUES(\"%s\", \"%s\", %d, \"%s\")";

MsgDumperSql::MsgDumperSql() :
	connection(NULL),
	table_created(false)
{

}

MsgDumperSql::~MsgDumperSql()
{
	WRITE_DEBUG_SYSLOG("The destructor of MsgDumperSql is called...");
}

unsigned short MsgDumperSql::try_connect_mysql()
{
	WRITE_DEBUG_SYSLOG("Initialize the parameters connected to the MySQL database server");
	connection = mysql_init(NULL); // 初始化数据库连接变量
	if(connection == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_init() fails, due to: %s", mysql_error(connection));
		return MSG_DUMPER_FAILURE_MYSQL;
	}

//	if (connect == NULL)
//	{
//		WRITE_ERR_SYSLOG("The parameters connected to the MySQL database server is NOT initialized...");
//		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
//	}

	WRITE_DEBUG_SYSLOG("Try to connect to the MySQL database server...");
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

		snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_create_database, database);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to create database[%s] by command: %s", database, cmd_buf);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_query() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The %s database is created", database);
	}
	WRITE_DEBUG_SYSLOG("Try to connect to the MySQL database server...... Successfully");

// Select the database
	if (mysql_select_db(connection, database))
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_select_db() fails, due to: %s", mysql_error(connection));
		return MSG_DUMPER_FAILURE_MYSQL;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::create_device_file()
{
// Create the connection to the MySQL server
	unsigned short ret = try_connect_mysql();
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

//	mysql_close(connection);
//	connection = NULL;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::write_device_file()
{
// Check if the connection is established
	if (connection == NULL)
	{
		WRITE_ERR_SYSLOG("The connection is NOT established");
		return MSG_DUMPER_FAILURE_MYSQL;
	}

// Checks to see if the connection to the MySQL server is still alive. If it is not, the client will attempt to reconnect automatically.
// This function returns zero if the connection is alive and nonzero in the case of an error.
	if (mysql_ping(connection))
	{
		WRITE_INFO_SYSLOG("The connection is NOT alive.Attempt to reconnect it......");
// Select the database
		if (mysql_select_db(connection, database))
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_select_db() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
	}

	if (!table_created)
	{
// Get the current time
		generate_current_time_string(current_time_string);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Current time string1: %s", current_time_string);

// Create the table in the database...
		snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_create_table, current_time_string);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Current time string2: %s", current_time_string);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to create table[sql%s] by command: %s", current_time_string, cmd_buf);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Current time string3: %s", current_time_string);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_query() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
		table_created = true;
	}

// Write the message into the log file
	for (int i = 0 ; i < write_vector.size() ; i++)
	{
		write_vector[i]->create_format_message(format_message, MSG_DUMPER_LONG_STRING_SIZE);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "CurTimeString: %s, Date: %s, Time: %s, Severity: %d, Data: %s", current_time_string, write_vector[i]->date_str, write_vector[i]->time_str, write_vector[i]->severity, write_vector[i]->data);
		snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_insert_into_table, current_time_string, write_vector[i]->date_str, write_vector[i]->time_str, write_vector[i]->severity, write_vector[i]->data);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Try to Write the message[%s] to MySQL by command: %s", format_message, cmd_buf);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "mysql_query() fails, due to: %s", mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
// Release the resource
		delete[] write_vector[i];
		write_vector[i] = NULL;
	}
// Clean-up the container
	write_vector.clear();

// Close the MySQL
//	mysql_close(connection);

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
	WRITE_DEBUG_SYSLOG("Release the parameters connected to the MySQL database server");
	if(connection != NULL)  // 关闭数据库连接
	{
		mysql_close(connection);
		connection = NULL;
	}

	return MsgDumperTimerThread::deinitialize();
}
