#ifndef MSG_DUMPER_MGR_H
#define MSG_DUMPER_MGR_H

#include "msg_dumper_base.h"


class MsgDumperMgr
{
private:
	bool is_init;
	unsigned short dumper_severity;
	unsigned short dumper_facility;

	MsgDumperBase* msg_dumper[2];

public:
	MsgDumperMgr();

	unsigned short initialize();
	unsigned short set_severity(unsigned short severity);
	unsigned short set_facility(unsigned short facility);
	unsigned short write_msg(unsigned short severity, const char* msg);
	unsigned short deinitialize();
};

#endif
