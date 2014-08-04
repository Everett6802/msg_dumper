#ifndef MSG_DUMPER_BASE_H
#define MSG_DUMPER_BASE_H

#include <stdio.h>
#include <stdlib.h>


class MsgDumperBase
{
public:
	MsgDumperBase();
	virtual ~MsgDumperBase(){}

	virtual unsigned short initialize(void* config=NULL)=0;
	virtual unsigned short write_msg(const char* msg)=0;
	virtual unsigned short deinitialize()=0;
};

#endif
