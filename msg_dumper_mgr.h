#ifndef MSG_DUMPER_MGR_H
#define MSG_DUMPER_MGR_H

#include <assert.h>
#include <map>
#include <string>
#include <stdexcept>
#include "common.h"
#include "msg_dumper.h"
#include "msg_dumper_timer_thread.h"


class MsgDumperMgr
{
private:
	struct MsgDumperFacilityFactory;
	MsgDumperFacilityFactory* facility_factory;

	typedef std::map<short, MSG_DUMPER_FACILITY> facility_map_type;
	facility_map_type facility_mapping_table;

//	static int facility_name_size;
	static short facility_flag[];
	static int facility_flag_size;

	char current_working_directory[MSG_DUMPER_STRING_SIZE];
	bool is_init;
	unsigned short dumper_severity_arr[FACILITY_SIZE];
	unsigned short dumper_facility;

	MsgDumperTimerThread* msg_dumper_thread[FACILITY_SIZE];

	int get_facility_index(unsigned short msg_dumper_facility_flag)const;

public:
	MsgDumperMgr();
	~MsgDumperMgr();

	bool can_ignore(unsigned short severity)const;

	unsigned short initialize();
	unsigned short set_severity(unsigned short facility, unsigned short single_severity);
	unsigned short set_severity_all(unsigned short facility);
	unsigned short set_facility(unsigned short facility);
	unsigned short get_severity(unsigned short single_facility)const;
	unsigned short get_facility()const;
	unsigned short write_msg(unsigned short severity, const char* msg);
	unsigned short deinitialize();
};

#endif
