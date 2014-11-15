#ifndef MSG_DUMPER_MGR_H
#define MSG_DUMPER_MGR_H

#include <map>
#include <string>
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
};

class MsgDumperMgr
{
private:
	static char* dev_name[];
	static short dev_flag[];

	factory device_factory;
	bool is_init;
	unsigned short dumper_severity;
	unsigned short dumper_facility;

	MsgDumperBase* msg_dumper[FACILITY_SIZE];


public:
	MsgDumperMgr();

	unsigned short initialize();
	unsigned short set_severity(unsigned short severity);
	unsigned short set_facility(unsigned short facility);
	unsigned short write_msg(unsigned short severity, const char* msg);
	unsigned short deinitialize();
};

#endif
