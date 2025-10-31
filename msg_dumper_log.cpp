#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msg_dumper.h"
#include "msg_dumper_log.h"


//#define DEBUG_ROTATE

using namespace std;

const unsigned long MsgDumperLog::KILO_BYTES = 1024;
const unsigned long MsgDumperLog::MEGA_BYTES = 1024 * KILO_BYTES;
const unsigned long MsgDumperLog::DEF_LOG_FILE_MAX_SIZE = 2 * MEGA_BYTES;
const int MsgDumperLog::DEF_LOG_FILE_ROTATE_AMOUNT = 10;
const int MsgDumperLog::DEF_LOG_FILE_ROTATE_TIMEDIFF = 1;
const int MsgDumperLog::DEF_LOG_FILE_ROTATE_CHECK_COUNT = 5;
const char* MsgDumperLog::DEF_LOG_CONFIG_FOLDERNAME = "log";
const char* MsgDumperLog::DEF_LOG_FILENAME = "dumper.log";
const char* MsgDumperLog::DEF_LOG_TAR_FILENAME_FORMAT ="dumper.tar.gz.%d";

MsgDumperLog::MsgDumperLog() :
	log_rotation_type(LOG_ROTATION_NONE),
	log_file_rotate_amount(DEF_LOG_FILE_ROTATE_AMOUNT),
	log_file_rotate_filesize(DEF_LOG_FILE_MAX_SIZE),
	rotate_check_count(0),
	fp_log(NULL),
	log_file_rotate_count(0),
	init_log_file_rotate_timestamp(false)
{
	memcpy(facility_name, MSG_DUMPER_FACILITY_DESC[FACILITY_LOG], strlen(MSG_DUMPER_FACILITY_DESC[FACILITY_LOG]));
	memset(log_foldername, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	memcpy(log_foldername, DEF_LOG_CONFIG_FOLDERNAME, sizeof(char) * strlen(DEF_LOG_CONFIG_FOLDERNAME));
	memset(log_folderpath, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
	memset(log_filepath, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
}

MsgDumperLog::~MsgDumperLog()
{
	if (fp_log != NULL)
	{
		fclose(fp_log);
		fp_log = NULL;
	}
//	deinitialize();
}

unsigned short MsgDumperLog::create_log_folder()
{
	// char folder_path[MSG_DUMPER_LONG_STRING_SIZE];
	// snprintf(folder_path, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s", current_working_directory, log_folder);
	struct stat st = {0};
// Check if the log folder exists or not
	if (stat(log_folderpath, &st) == -1)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Try to create a log folder: %s", log_folderpath);
// If not, create a new folder
		if (mkdir(log_folderpath, 0744) != 0)
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to create a log folder[%s], due to %s", log_folderpath, strerror(errno));
			return MSG_DUMPER_FAILURE_UNKNOWN;
		}
	}
	else
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "The log folder[%s] has already existed", log_folderpath);
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::parse_config_param(const char* param_title, const char* param_content)
{
	if (param_title == NULL || param_content == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid argument: param_title/param_content");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	static const char* title[] = {"log_folder", "log_rotation"};
	static int title_len = sizeof title / sizeof title[0];

	unsigned short ret = MSG_DUMPER_SUCCESS;
	bool found = false;
	for (int index = 0 ; index < title_len ; index++)
	{
		if (strcmp(title[index], param_title) == 0)
		{
			int param_content_len = strlen(param_content);
			switch(index)
			{
			case 0:
				memset(log_foldername, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
				memcpy(log_foldername, param_content, param_content_len);
				break;
			case 1:
				log_rotation_type = (LogRotationType)atoi(param_content);
				if (log_rotation_type < 0 || log_rotation_type >= LOG_ROTATION_SIZE)
				{
					WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect parameter, unknown log rotation type: %s", param_content);
					return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
				}
				break;
			default:
// Impossible
				assert(false);
			}
			found = true;
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

unsigned short MsgDumperLog::open_device()
{
// Open the file
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Open the log file: %s", log_filename);
	fp_log = fopen(log_filepath, "a+");
	if (fp_log == NULL)
	{
		WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "open() fails, due to: %s", strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::close_device()
{
// Close the file
	if (fp_log != NULL)
	{
		fclose(fp_log);
		fp_log = NULL;
	}

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::initialize(const char* current_working_directory, void* config)
{
	WRITE_DEBUG_SYSLOG("Initialize the MsgDumperLog object......");

// Parse the config file first
	unsigned short ret = parse_config(current_working_directory, "log");
	if (CHECK_FAILURE(ret))
		return ret;
// Modify the member variables after parsing the config
	snprintf(log_folderpath, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s", current_working_directory, log_foldername);
	// printf("log_folderpath: %s, %d\n", log_folderpath, strlen(log_folderpath));
	snprintf(log_filepath, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s", log_folderpath, DEF_LOG_FILENAME);
	// printf("log_filepath: %s, %d\n", log_filepath, strlen(log_filepath));
	snprintf(log_tar_filepath_format, MSG_DUMPER_LONG_STRING_SIZE, "%s/%s", log_folderpath, DEF_LOG_TAR_FILENAME_FORMAT);
	snprintf(log_tar_file_count_cmd, MSG_DUMPER_LONG_STRING_SIZE, "ls -al %s | grep tar.gz | wc -l", log_folderpath);
// Create the log folder
	ret = create_log_folder();
	if (CHECK_FAILURE(ret))
		return ret;
// Initialize the parameters
	log_file_rotate_count = calculate_log_tar_file_count();
#ifdef DEBUG_ROTATE
	fprintf(stderr, "Rotation Count: %d\n", log_file_rotate_count);
#endif
	WRITE_DEBUG_FORMAT_SYSLOG("The log file rotation count: %d", log_file_rotate_count);

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::deinitialize()
{
	WRITE_DEBUG_SYSLOG("DeInitialize the MsgDumperLog object......");

	return close_device();
}

unsigned long MsgDumperLog::get_log_file_size()const
{
	static struct stat st;
	static bool log_file_exist;
	if (fp_log != NULL)
		log_file_exist = (fstat(fileno(fp_log), &st) == 0);
	else
		log_file_exist = (stat(log_filepath, &st) == 0);
	assert(log_file_exist && "The log file does NOT exist");
	return st.st_size;
}

bool MsgDumperLog::check_log_file_exist()const
{
	static struct stat st;
	bool log_file_exist = 0;
	if (fp_log != NULL)
		log_file_exist = (fstat(fileno(fp_log), &st) == 0);
	else
		log_file_exist = (stat(log_filepath, &st) == 0);
	return log_file_exist;
}

unsigned short MsgDumperLog::reset_log_file_content()
{
#ifdef DEBUG_ROTATE
    fprintf(stderr, "Reset: %s\n", DEF_LOG_FILENAME);
#endif
    if (!check_log_file_exist())
    {
    	WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "The log file[%s] does NOT exist", log_filepath);
    	return MSG_DUMPER_FAILURE_NOT_FOUND;
    }
    unsigned short ret = MSG_DUMPER_SUCCESS;
    ret = close_device();
    if (CHECK_FAILURE(ret))
    	return ret;
#ifdef DEBUG_ROTATE
    fprintf(stderr, "Remove old log file: %s\n", DEF_LOG_FILENAME);
#endif
    unlink(log_filepath);
    ret = open_device();
    if (CHECK_FAILURE(ret))
    	return ret;
    // fseek(fp_log, 0, SEEK_SET);
    // rewind(fp_log);
    return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::get_log_tar_filename(int index, string& log_tar_filename)const
{
// log tar index range [1, log_file_rotate_amount]
	if (index < 1 || index > log_file_rotate_amount)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log rotation index[%d] is Out of Range [0, %d)", index, log_file_rotate_amount);
		return MSG_DUMPER_FAILURE_OUT_OF_RANGE;
	}
	static char log_tar_filename_tmp[MSG_DUMPER_STRING_SIZE];
	snprintf(log_tar_filename_tmp, MSG_DUMPER_STRING_SIZE, DEF_LOG_TAR_FILENAME_FORMAT, index);
	log_tar_filename.assign(log_tar_filename_tmp);
    return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::get_log_tar_filepath(int index, string& log_tar_filepath)const
{
// log tar index range [1, log_file_rotate_amount]
	if (index < 1 || index > log_file_rotate_amount)
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "The log rotation index[%d] is Out of Range [0, %d)", index, log_file_rotate_amount);
		return MSG_DUMPER_FAILURE_OUT_OF_RANGE;
	}
	static char log_tar_filepath_tmp[MSG_DUMPER_LONG_STRING_SIZE];
	snprintf(log_tar_filepath_tmp, MSG_DUMPER_LONG_STRING_SIZE, log_tar_filepath_format, index);
	log_tar_filepath.assign(log_tar_filepath_tmp);
    return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::calculate_log_tar_file_count()const
{
	FILE *fp = popen(log_tar_file_count_cmd, "r");
	if (fp == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG("popen() fails, due to: %s", cmd, strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
// Caution: Nothing to read
	static char buf[MSG_DUMPER_SHORT_STRING_SIZE];
	if(fgets(buf, MSG_DUMPER_SHORT_STRING_SIZE, fp) == NULL) 
	{
		WRITE_ERR_FORMAT_SYSLOG("fgets() fails, due to: %s", cmd, strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	pclose(fp);
    return atoi(buf);
}

unsigned short MsgDumperLog::rotate_log_tar_file()
{
	unsigned short ret =  MSG_DUMPER_SUCCESS;
#ifdef DEBUG_ROTATE
	fprintf(stderr, "rotate_count: %d, rotate_amount: %d\n", log_file_rotate_count, log_file_rotate_amount);
#endif
// Need to delete the oldest file
    if (log_file_rotate_count == log_file_rotate_amount)
    {
    	string log_tar_filepath;
    	ret = get_log_tar_filepath(log_file_rotate_amount, log_tar_filepath);
    	if (CHECK_FAILURE(ret))
    		return ret;
    	unlink(log_tar_filepath.c_str());
    }
// Shift the rar file
    int rotate_count = ((log_file_rotate_count == log_file_rotate_amount) ? (log_file_rotate_amount - 1) : log_file_rotate_count);
    for (int index = rotate_count ; index >= 1 ; index--)
    {
    	string old_log_tar_filepath;
    	ret = get_log_tar_filepath(index, old_log_tar_filepath);
    	if (CHECK_FAILURE(ret))
    		return ret;
    	string new_log_tar_filepath;
    	ret = get_log_tar_filepath(index + 1, new_log_tar_filepath);
    	if (CHECK_FAILURE(ret))
    		return ret;
#ifdef DEBUG_ROTATE
    	fprintf(stderr, "Rename: %s->%s\n", old_log_tar_filepath.c_str(), new_log_tar_filepath.c_str());
#endif
		if (rename(old_log_tar_filepath.c_str(), new_log_tar_filepath.c_str()) == -1)
		{
			WRITE_ERR_FORMAT_SYSLOG("rename() fails, due to: %s", cmd, strerror(errno));
			return MSG_DUMPER_FAILURE_SYSTEM_API;
		}
    }
// Tar the current file
    string log_tar_filename;
    ret = get_log_tar_filename(1, log_tar_filename);
    if (CHECK_FAILURE(ret))
    	return ret;
#ifdef DEBUG_ROTATE
    fprintf(stderr, "Compress: %s->%s\n", DEF_LOG_FILENAME, log_tar_filename.c_str());
#endif
    ret = compress_log_file(DEF_LOG_FILENAME, log_tar_filename.c_str());
    if (CHECK_FAILURE(ret))
    	return ret;
#ifdef DEBUG_ROTATE
    fprintf(stderr, "Compress... Done\n");
#endif
// Add the log count
    if (log_file_rotate_count < log_file_rotate_amount)
    	log_file_rotate_count += 1;
// Re-generate the log file
    ret = reset_log_file_content();
   	if (CHECK_FAILURE(ret))
    	return ret;
    return MSG_DUMPER_SUCCESS;
}


unsigned short MsgDumperLog::compress_log_file(const char* filename, const char* tar_filename)const
{
	assert(filename != NULL && tar_filename != NULL && "filename is NULL or tar_filename is NULL");
	static const int BUF_SIZE = 256;
	static char cmd[BUF_SIZE];
	// static char errmsg[BUF_SIZE];
// Change the working folder
#ifdef DEBUG_ROTATE
	fprintf(stderr, "chdir: %s\n", log_folderpath);
#endif
	if (chdir(log_folderpath) == -1)
	{
		WRITE_ERR_FORMAT_SYSLOG("chdir() fails, due to: %s", strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;		
	}
	FILE* fp_cmd = NULL;
// Tar the file
	snprintf(cmd, BUF_SIZE, "tar czf %s %s", tar_filename, filename);
#ifdef DEBUG_ROTATE
	fprintf(stderr, "Tar cmd: %s\n", cmd);
#endif
	fp_cmd = popen(cmd, "r");
	if (fp_cmd == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG("The command[%s] fails, due, to: %s", cmd, strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	pclose(fp_cmd);
	fp_cmd = NULL;
// Check the tar exist
	snprintf(cmd, BUF_SIZE, "find . -name '%s' | wc -l", tar_filename);
#ifdef DEBUG_ROTATE
	fprintf(stderr, "Check tar cmd: %s\n", cmd);
#endif
	fp_cmd = popen(cmd, "r");
	if (fp_cmd == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG("The command[%s] fails, due, to: %s", cmd, strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	static char buf[BUF_SIZE];
	if (fgets(buf, BUF_SIZE, fp_cmd) == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG("fgets() due, to: %s", strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	pclose(fp_cmd);
	fp_cmd = NULL;
	if (atoi(buf) != 1)
	{
		WRITE_ERR_FORMAT_SYSLOG("The result[%s] is incorrect from the cmd: %s", buf, cmd);
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::uncompress_log_tar_file(const char* tar_filename)const
{
	assert(tar_filename != NULL && "tar_filename is NULL");
	static const int BUF_SIZE = 256;
	static char cmd[BUF_SIZE];
	// static char errmsg[BUF_SIZE];
// Change the working folder
	if (chdir(log_folderpath) == -1)
	{
		WRITE_ERR_FORMAT_SYSLOG("chdir() fails, due, to: %s", strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;		
	}
	FILE* fp_cmd = NULL;
// Tar the file
	snprintf(cmd, BUF_SIZE, "tar xzf %s", tar_filename);
	fp_cmd = popen(cmd, "r");
	if (fp_cmd == NULL)
	{
		WRITE_ERR_FORMAT_SYSLOG("The command[%s] fails, due, to: %s", cmd, strerror(errno));
		return MSG_DUMPER_FAILURE_SYSTEM_API;
	}
	pclose(fp_cmd);
	fp_cmd = NULL;
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_msg_log_rotation_none(const char* msg_str)
{
	fputs(msg_str, fp_log);
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperLog::write_msg_log_rotation_filesize(const char* msg_str)
{
// Check if it's requried to rotate the file
// #ifdef DEBUG_ROTATE
// 	fprintf(stderr, "Msg: %s\n", msg_str);
// #endif
	if (rotate_check_count != DEF_LOG_FILE_ROTATE_CHECK_COUNT)
	{
// It's NOT required to check rotation every time
		rotate_check_count++;
	}
	else
	{
		rotate_check_count = 0;
		unsigned long log_filesize = get_log_file_size();
#ifdef DEBUG_ROTATE
		fprintf(stderr, "Log FileSize: %ld\n", log_filesize);
#endif
		bool need_rotate = ((log_filesize >= log_file_rotate_filesize) ? true : false);
		unsigned short ret = MSG_DUMPER_SUCCESS;
		if (need_rotate)
		{
#ifdef DEBUG_ROTATE
			fprintf(stderr, "Need Rotate from FileSize\n");
#endif
			ret = rotate_log_tar_file();
			if (CHECK_FAILURE(ret))
				return ret;
		}
	}
	return write_msg_log_rotation_none(msg_str);
}

unsigned short MsgDumperLog::write_msg_log_rotation_timestamp(const char* msg_str)
{
// Check if it's requried to rotate the file
// #endif
	if (rotate_check_count != DEF_LOG_FILE_ROTATE_CHECK_COUNT)
	{
// It's NOT required to check rotation every time
		rotate_check_count++;
	}
	else
	{
		bool need_rotate = false;
		static time_t cur_timestamp;
		time(&cur_timestamp); 
		if (init_log_file_rotate_timestamp)
		{
			if (difftime(cur_timestamp, log_file_rotate_timestamp) > DEF_LOG_FILE_ROTATE_TIMEDIFF)
				need_rotate = true;
// #ifdef DEBUG_ROTATE
// 		static char buf[256];
// 		struct tm* time1 = localtime(&cur_timestamp);
// 		strftime(buf, 256, "%Y:%m:%d %H:%M:%S", time1);
// 		fprintf(stderr, "time1: %s\n", buf);
// 		struct tm* time2 = localtime(&log_file_rotate_timestamp);
// 		strftime(buf, 256, "%Y:%m:%d %H:%M:%S", time2);
// 		fprintf(stderr, "time2: %s\n", buf);
// 		fprintf(stderr, "diff: %lf, thres: %d\n", difftime(cur_timestamp, log_file_rotate_timestamp), DEF_LOG_FILE_ROTATE_TIMEDIFF);
// #endif
		}
		else
			init_log_file_rotate_timestamp = true;
		memcpy(&log_file_rotate_timestamp, &cur_timestamp, sizeof(time_t));

		unsigned short ret = MSG_DUMPER_SUCCESS;
		if (need_rotate)
		{
#ifdef DEBUG_ROTATE
			fprintf(stderr, "Need Rotate from TimeStamp\n");
#endif
			ret = rotate_log_tar_file();
			if (CHECK_FAILURE(ret))
				return ret;
		}
	}
	return write_msg_log_rotation_none(msg_str);
}

unsigned short MsgDumperLog::write_msg(PMSG_CFG msg_cfg)
{
	typedef unsigned short (MsgDumperLog::*WRITE_MSG_LOG_ROTATION_FUNCPTR)(const char* msg_str);
	static WRITE_MSG_LOG_ROTATION_FUNCPTR write_msg_log_func_array[] =
	{
		&MsgDumperLog::write_msg_log_rotation_none,
		&MsgDumperLog::write_msg_log_rotation_filesize,
		&MsgDumperLog::write_msg_log_rotation_timestamp
	};
// Write the message into the log file
	// WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write the message[%s] to file [%s]", msg_cfg->to_string(), log_filename);
	return (this->*(write_msg_log_func_array[log_rotation_type]))(msg_cfg->to_string());
}
