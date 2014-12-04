#include <fcntl.h>
#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_sql.h"

/*
 * Go to the following links to see more detailed info:
 * http://www.cs.wichita.edu/~chang/lecture/cs742/program/how-mysql-c-api.html
 * http://docstore.mik.ua/orelly/linux/sql/ch19_01.htm
 */

char* MsgDumperSql::DEF_SERVER = "localhost";
char* MsgDumperSql::DEF_USERNAME = "root";
char* MsgDumperSql::DEF_PASSWORD = "lab4man1";
char* MsgDumperSql::DEF_DATABASE = "msg_dumper";
char* MsgDumperSql::format_cmd_create_database = "CREATE DATABASE %s";
char* MsgDumperSql::format_cmd_create_table = "CREATE TABLE sql%s (date VARCHAR(16), time VARCHAR(16), severity INT, data VARCHAR(512))";
char* MsgDumperSql::format_cmd_insert_into_table = "INSERT INTO sql%s VALUES(\"%s\", \"%s\", %d, \"%s\")";

MsgDumperSql::MsgDumperSql() :
	connection(NULL),
	table_created(false)
{
	snprintf(worker_thread_name, MSG_DUMPER_SHORT_STRING_SIZE, "SQL");
	memcpy(server, DEF_SERVER, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(username, DEF_USERNAME, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(password, DEF_PASSWORD, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(database, DEF_DATABASE, sizeof(char) * MSG_DUMPER_STRING_SIZE);
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
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The connection is NOT established", get_thread_name());
		return MSG_DUMPER_FAILURE_MYSQL;
	}

// Checks to see if the connection to the MySQL server is still alive. If it is not, the client will attempt to reconnect automatically.
// This function returns zero if the connection is alive and nonzero in the case of an error.
	if (mysql_ping(connection))
	{
		WRITE_INFO_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The connection is NOT alive.Attempt to reconnect it......", get_thread_name());
// Select the database
		if (mysql_select_db(connection, database))
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> mysql_select_db() fails, due to: %s", get_thread_name(), mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
	}

	if (!table_created)
	{
// Get the current time
		generate_current_time_string(current_time_string);

// Create the table in the database...
		snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_create_table, current_time_string);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Thread[%s]=> Try to create table[sql%s] by command: %s", get_thread_name(), current_time_string, cmd_buf);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			int error = mysql_errno(connection);
			if (error != 1050)
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> mysql_query() fails, due to: %d, %s", get_thread_name(), error, mysql_error(connection));
				return MSG_DUMPER_FAILURE_MYSQL;
			}
			else
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> The sql%s has already existed", get_thread_name(), current_time_string);
		}
		table_created = true;
	}

// Write the message into the log file
	for (int i = 0 ; i < write_vector.size() ; i++)
	{
		PMSG_CFG msg_cfg = write_vector[i];
		snprintf(cmd_buf, MSG_DUMPER_LONG_STRING_SIZE, format_cmd_insert_into_table, current_time_string, msg_cfg->date_str, msg_cfg->time_str, msg_cfg->severity, msg_cfg->data);
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Thread[%s]=> Try to Write the message[%s] to MySQL by command: %s", get_thread_name(), msg_cfg->to_string(), cmd_buf);
		if(mysql_query(connection, cmd_buf) != NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Thread[%s]=> mysql_query() fails, due to: %s", get_thread_name(), mysql_error(connection));
			return MSG_DUMPER_FAILURE_MYSQL;
		}
// Release the resource
		delete[] msg_cfg;
		write_vector[i] = NULL;
	}
// Clean-up the container
	write_vector.clear();

// Close the MySQL
//	mysql_close(connection);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperSql::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static char* title[] = {"server", "username", "password", "database"};
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
				param_member_variable = server;
				break;
			case 1:
				param_member_variable = username;
				break;
			case 2:
				param_member_variable = password;
				break;
			case 3:
				param_member_variable = database;
				break;
			}

			if (param_member_variable != NULL)
			{
				memset(param_member_variable, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
				memcpy(param_member_variable, param_content, param_content_len);
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

unsigned short MsgDumperSql::initialize(void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperSql object......");

// Parse the config file first
	unsigned short ret = parse_config("sql");
	if (CHECK_MSG_DUMPER_FAILURE(ret))
		return ret;

// Create the specific table
	ret = create_device_file();
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
