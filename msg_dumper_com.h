#ifndef MSG_DUMPER_COM_H
#define MSG_DUMPER_COM_H

#include <termios.h>
#include <unistd.h>
//#include <iostream>
//#include <vector>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"

//using namespace std;


class MsgDumperCom : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	static char* COM_PORT_NAME;
	static speed_t COM_PORT_SPEED;

	MsgDumperCom();
	~MsgDumperCom();

	unsigned short try_open_comport(int& fd_com)const;

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();

public:
	virtual unsigned short initialize(void* config=NULL);
};

#endif
