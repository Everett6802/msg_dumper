#ifndef MSG_DUMPER_LOG_H
#define MSG_DUMPER_LOG_H

#include <stdio.h>
#include <time.h>
#include <string>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperLog : public MsgDumperBase
{
	enum LogRotationType{LOG_ROTATION_NONE, LOG_ROTATION_FILESIZE, LOG_ROTATION_TIMESTAMP, LOG_ROTATION_SIZE};
private:
	static const unsigned long KILO_BYTES;
	static const unsigned long MEGA_BYTES;
	static const unsigned long DEF_LOG_FILE_MAX_SIZE;
	static const int DEF_LOG_FILE_ROTATE_AMOUNT;
	static const int DEF_LOG_FILE_ROTATE_TIMEDIFF;
	static const int DEF_LOG_FILE_ROTATE_CHECK_COUNT;
	static const char* DEF_LOG_CONFIG_FOLDERNAME;
	static const char* DEF_LOG_FILENAME;
	static const char* DEF_LOG_TAR_FILENAME_FORMAT;

	// char* log_filename;
	// char* log_filepath;
// Probably modified from the config file
	char log_foldername[MSG_DUMPER_STRING_SIZE];
	LogRotationType log_rotation_type;
	int log_file_rotate_amount;
	unsigned long log_file_rotate_filesize;
	int rotate_check_count;

	char log_folderpath[MSG_DUMPER_LONG_STRING_SIZE];
	char log_filepath[MSG_DUMPER_LONG_STRING_SIZE];
	char log_tar_filename_format[MSG_DUMPER_STRING_SIZE];
	char log_tar_filepath_format[MSG_DUMPER_LONG_STRING_SIZE];
	char log_tar_file_count_cmd[MSG_DUMPER_LONG_STRING_SIZE];
	FILE* fp_log;
	int log_file_rotate_count;
	bool init_log_file_rotate_timestamp;
	time_t log_file_rotate_timestamp; 

	unsigned short create_log_folder();
	unsigned long get_log_file_size()const;
	bool check_log_file_exist()const;
	unsigned short reset_log_file_content();
	unsigned short get_log_tar_filename(int index, std::string& log_tar_filename)const;
	unsigned short get_log_tar_filepath(int index, std::string& log_tar_filepath)const;
	unsigned short calculate_log_tar_file_count()const;
	unsigned short rotate_log_tar_file();
	unsigned short compress_log_file(const char* filename, const char* tar_filename)const;
	unsigned short uncompress_log_tar_file(const char* tar_filename)const;
	unsigned short write_msg_log_rotation_none(const char* msg_str);
	unsigned short write_msg_log_rotation_filesize(const char* msg_str);
	unsigned short write_msg_log_rotation_timestamp(const char* msg_str);

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperLog();
	virtual ~MsgDumperLog();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* config_path, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
