#ifndef MSG_DUMPER_BASE_H
#define MSG_DUMPER_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


class MsgDumperBase
{
protected:
	bool device_handle_exist;

public:
	MsgDumperBase();
	virtual ~MsgDumperBase(){}

	virtual unsigned short initialize(void* config=NULL)=0;
	virtual unsigned short deinitialize()=0;
	virtual unsigned short write_msg(const time_t& timep, unsigned short severity, const char* msg)=0;
};

#endif
