#ifndef MSG_DUMPER_MGR_H
#define MSG_DUMPER_MGR_H

#include <assert.h>
#include <map>
#include <string>
#include <stdexcept>
#include "msg_dumper.h"
#include "msg_dumper_base.h"


using namespace std;
#define REGISTER_CLASS(n) device_factory.register_class<n>(#n)

template <class T> MsgDumperBase* constructor() { return (MsgDumperBase*)new T(); }

// A simple factory. To allocate a memory by sending the type name as an argument
struct factory
{
	typedef MsgDumperBase*(*constructor_t)();
	typedef map<string, constructor_t> map_type;
	map_type m_classes;

	template <class T>
	void register_class(string const& n){ m_classes.insert(make_pair(n, &constructor<T>));}

	MsgDumperBase* construct(std::string const& n)
	{
		map_type::iterator i = m_classes.find(n);
		if (i == m_classes.end())
			return NULL; // or throw or whatever you want
		return (MsgDumperBase*)i->second(); // Allocate the memory of a specific type
	}

	int register_class_size()const{return m_classes.size();}
};

class MsgDumperMgr
{
private:
	enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_SQL, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_SIZE};
	typedef map<short, MSG_DUMPER_FACILITY> facility_map_type;
	facility_map_type facility_mapping_table;

	static char* dev_name[];
	static int dev_name_size;
	static short dev_flag[];
	static int dev_flag_size;

	char current_working_directory[MSG_DUMPER_STRING_SIZE];
	factory device_factory;
	bool is_init;
	unsigned short dumper_severity_arr[FACILITY_SIZE];
	unsigned short dumper_facility;

	MsgDumperBase* msg_dumper[FACILITY_SIZE];

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
