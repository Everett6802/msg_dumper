#ifndef MSG_DUMPER_BASE_H
#define MSG_DUMPER_BASE_H

class MsgDumperBase
{
public:
	virtual ~MsgDumperBase();

	virtual unsigned short initialize(void* config)=0;
	virtual unsigned short write_msg(const char* msg)=0;
	virtual unsigned short deinitialize()=0;
};

#endif
