#ifndef MSG_DUMPER_BASE_H
#define MSG_DUMPER_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <vector>
#include "common.h"


#define CURRENT_TIME_STRING_LENGTH 11

using namespace std;

class MsgDumperBase
{
private:
	static const char* CONF_FOLDER_NAME;
	unsigned short severity;

protected:
	bool device_handle_exist;
	char facility_name[MSG_DUMPER_SHORT_STRING_SIZE];

	unsigned short generate_current_time_string(char* current_time_string);
	unsigned short parse_config(const char* conf_path, const char* dev_name);

	unsigned short set_severity(unsigned short new_severity);
	unsigned short get_severity()const;

	virtual unsigned short parse_config_param(const char* param_title, const char* param_content)=0;

public:
	MsgDumperBase();
	virtual ~MsgDumperBase(){}

	const char* get_facility_name()const;

	virtual unsigned short open_device()=0;
	virtual unsigned short close_device()=0;

	virtual unsigned short initialize(const char* config_path, void* config=NULL)=0;
	virtual unsigned short deinitialize()=0;
	virtual unsigned short write_msg(PMSG_CFG msg_cfg)=0;
};

#endif
