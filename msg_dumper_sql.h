#ifndef MSG_DUMPER_SQL_H
#define MSG_DUMPER_SQL_H

#include <mysql/mysql.h>
#include "msg_dumper_timer_thread.h"
#include "msg_dumper_mgr.h"


class MsgDumperSql : public MsgDumperTimerThread
{
	friend class MsgDumperMgr;
private:
	unsigned short try_open_mysql(int& fd_com)const;

protected:
	virtual unsigned short create_device_file();
	virtual unsigned short write_device_file();

public:
	MsgDumperSql();
	~MsgDumperSql();

	virtual unsigned short initialize(void* config=NULL);
};

#endif
