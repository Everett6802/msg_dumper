#ifndef MSG_DUMPER_COM_H
#define MSG_DUMPER_COM_H

#include <termios.h>
#include <unistd.h>
#include "common.h"
#include "msg_dumper_base.h"


class MsgDumperCom : public MsgDumperBase
{
private:
	static const char* DEF_COM_PORT_NAME;
	static const char* DEF_COM_PORT_SPEED;

	int fd_com;
	char port_name[MSG_DUMPER_STRING_SIZE];
	char port_speed[MSG_DUMPER_STRING_SIZE];
	speed_t com_port_speed;
	struct termios tty_new;
	struct termios tty_original;

	unsigned short try_open_comport();
	unsigned short transform_com_port_speed(speed_t& com_port_speed)const;

protected:
	virtual unsigned short parse_config_param(const char* param_title, const char* param_content);

public:
	MsgDumperCom();
	virtual ~MsgDumperCom();

	virtual unsigned short open_device();
	virtual unsigned short close_device();

	virtual unsigned short initialize(const char* current_working_directory, void* config=NULL);
	virtual unsigned short deinitialize();
	virtual unsigned short write_msg(PMSG_CFG msg_cfg);
};

#endif
