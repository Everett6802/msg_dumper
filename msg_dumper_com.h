#ifndef MSG_DUMPER_COM_H
#define MSG_DUMPER_COM_H

#include <termios.h>
#include <unistd.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


class MsgDumperCom : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	static char* DEF_COM_PORT_NAME;
	static char* DEF_COM_PORT_SPEED;

	char port_name[MSG_DUMPER_STRING_SIZE];
	char port_speed[MSG_DUMPER_STRING_SIZE];

	unsigned short try_open_comport(int& fd_com)const;
	unsigned short transform_com_port_speed(speed_t& com_port_speed)const;

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperCom();
	virtual ~MsgDumperCom();

	virtual unsigned short initialize(void* config=NULL);
};

#endif
