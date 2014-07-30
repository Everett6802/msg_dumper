#include "msg_dumper_com.h"
#include "msg_dumper.h"

MsgDumperCom::MsgDumperCom()
{

}

unsigned short MsgDumperCom::initialize(void* config)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::write_msg(const char* msg)
{
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperCom::deinitialize()
{
	return MSG_DUMPER_SUCCESS;
}
