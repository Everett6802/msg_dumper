#ifndef MSG_DUMPER_BASE_H
#define MSG_DUMPER_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <vector>


#define CURRENT_TIME_STRING_LENGTH 11

using namespace std;

class MsgDumperBase
{
private:
	static const char* CONF_FOLDER_NAME;

protected:
	bool device_handle_exist;

	unsigned short generate_current_time_string(char* current_time_string);
	unsigned short parse_config(const char* conf_path, const char* dev_name);

	virtual unsigned short parse_config_param(const char* param_title, const char* param_content)=0;

public:
	MsgDumperBase();
	virtual ~MsgDumperBase(){}

	virtual unsigned short initialize(const char* config_path, void* config=NULL)=0;
	virtual unsigned short deinitialize()=0;
	virtual unsigned short write_msg(const time_t& timep, unsigned short severity, const char* msg)=0;
};

#endif
