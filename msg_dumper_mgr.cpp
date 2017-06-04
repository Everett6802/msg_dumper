#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"
#include "msg_dumper_sql.h"
#include "msg_dumper_remote.h"
#include "msg_dumper_syslog.h"


using namespace std;

//int MsgDumperMgr::facility_name_size = sizeof(MSG_DUMPER_FACILITY_DESC) / sizeof(MSG_DUMPER_FACILITY_DESC[0]);
short MsgDumperMgr::facility_flag[] = {MSG_DUMPER_FACILITY_LOG, MSG_DUMPER_FACILITY_COM, MSG_DUMPER_FACILITY_SQL, MSG_DUMPER_FACILITY_REMOTE, MSG_DUMPER_FACILITY_SYSLOG};
int MsgDumperMgr::facility_flag_size = sizeof(facility_flag) / sizeof(facility_flag[0]);

#define REGISTER_CLASS(n) facility_factory->register_class<n>(#n)

template <class T> MsgDumperBase* constructor() { return (MsgDumperBase*)new T(); }

// A simple factory. To allocate a memory by sending the type name as an argument
struct MsgDumperMgr::MsgDumperFacilityFactory
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
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to find the MsgDumper%s class", n.c_str());
			throw invalid_argument("Unknown class");
			return NULL; // or throw or whatever you want
		}

		MsgDumperBase* msg_dumper = (MsgDumperBase*)i->second();  // Allocate the memory of a specific type
		if (msg_dumper == NULL)
		{
			WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to allocate the MsgDumper%s object", n.c_str());
			throw bad_alloc();
		}
		return msg_dumper;
	}

	int register_class_size()const{return m_classes.size();}
};

MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_facility(MSG_DUMPER_FACILITY_LOG)
{
	facility_factory = new MsgDumperFacilityFactory();
	if (facility_factory == NULL)
		throw bad_alloc();

// Register the class to the simple factory
	REGISTER_CLASS(MsgDumperLog);
	REGISTER_CLASS(MsgDumperCom);
	REGISTER_CLASS(MsgDumperSql);
	REGISTER_CLASS(MsgDumperRemote);
	REGISTER_CLASS(MsgDumperSyslog);
// Check the parameter setting is correct
	assert((facility_factory->register_class_size() == FACILITY_SIZE) && "The facility size is NOT identical");
	assert((MSG_DUMPER_FACILITY_DESC_LEN == FACILITY_SIZE) && "The facility name size is NOT identical");
	assert((facility_flag_size == FACILITY_SIZE) && "The facility flag size is NOT identical");

	memset(current_working_directory, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		dumper_severity_arr[i] = MSG_DUMPER_SEVIRITY_ERROR;

// Create the facility mapping table
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		facility_mapping_table.insert(make_pair(facility_flag[i], (MSG_DUMPER_FACILITY)i));

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		msg_dumper_thread[i] = NULL;
}

MsgDumperMgr::~MsgDumperMgr()
{
}

bool MsgDumperMgr::can_ignore(unsigned short msg_dumper_severity)const
{
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper_thread[i] && msg_dumper_severity <= dumper_severity_arr[i])
			return false;
	}
	return true;
}

unsigned short MsgDumperMgr::initialize()
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL)
	{
		WRITE_DEBUG_SYSLOG("Fail to get the current working folder");
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Current working folder: %s", current_working_directory);

	unsigned short ret = MSG_DUMPER_SUCCESS;
	char facility_class_name[32];
// Initialize the device
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		snprintf(facility_class_name, 32, "MsgDumper%s", MSG_DUMPER_FACILITY_DESC[i]);
// Allocate and initialize the object derived from the MsgDumplerBase class
		if (dumper_facility & facility_flag[i])
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Allocate the %s object", facility_class_name);
			msg_dumper_thread[i] = new MsgDumperTimerThread(facility_factory->construct(facility_class_name));
			if (msg_dumper_thread[i] == NULL)
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to allocate the %s object", facility_class_name);
				return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
			}

			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Initialize the %s object", facility_class_name);
			ret = msg_dumper_thread[i]->initialize(current_working_directory);
			if (CHECK_FAILURE(ret))
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to initialize the %s object", facility_class_name);
				goto EXIT;
			}
		}
	}

	is_init = true;

EXIT:
	if (!is_init)
	{
		for (int i = 0 ; i < FACILITY_SIZE ; i++)
		{
			if (msg_dumper_thread[i] != NULL)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Deinitialize the MsgDumper%s object due to the failure of initialization of some objects", MSG_DUMPER_FACILITY_DESC[i]);
				msg_dumper_thread[i]->deinitialize();
				delete msg_dumper_thread[i];
				msg_dumper_thread[i] = NULL;
			}
		}
	}
	return ret;
}

int MsgDumperMgr::get_facility_index(unsigned short msg_dumper_facility_flag)const
{
	if (!(MSG_DUMPER_FACILITY_ALL & msg_dumper_facility_flag))
	{
		WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Incorrect facility flag: %d", msg_dumper_facility_flag);
		throw invalid_argument("Incorrect facility flag");
	}

	return (int)facility_mapping_table.find(msg_dumper_facility_flag)->second;
}

unsigned short MsgDumperMgr::set_severity(unsigned short facility, unsigned short single_msg_dumper_severity)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	int facility_index = (int)get_facility_index(facility);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity[%d] to facility[%s]", single_msg_dumper_severity, MSG_DUMPER_FACILITY_DESC[facility_index]);
	dumper_severity_arr[facility_index] = single_msg_dumper_severity;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::set_severity_all(unsigned short msg_dumper_severity)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity[%d] to all facilities", msg_dumper_severity);
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		dumper_severity_arr[i] = msg_dumper_severity;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::set_facility(unsigned short facility)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the facility to %d", facility);
	dumper_facility = facility;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::get_severity(unsigned short single_facility)const
{
	int facility_index = (int)get_facility_index(single_facility);
	unsigned short msg_dumper_severity = dumper_severity_arr[facility_index];
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Get the severity of facility[%s]: %d", MSG_DUMPER_FACILITY_DESC[facility_index], msg_dumper_severity);
	return msg_dumper_severity;
}

unsigned short MsgDumperMgr::get_facility()const
{
	return dumper_facility;
}

unsigned short MsgDumperMgr::write_msg(unsigned short msg_dumper_severity, const char* msg)
{
	if (!is_init)
	{
		WRITE_ERR_SYSLOG("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	if (msg == NULL)
	{
		WRITE_ERR_SYSLOG("Invalid pointer: msg");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
// Mark the time that the message is generated
	time_t timep;
	time(&timep);

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		// fprintf(stderr, "MsgDumperMgr::write_msg [severity: %d, message: %s]\n", severity, msg);
		if (msg_dumper_thread[i] && msg_dumper_severity <= dumper_severity_arr[i])
		{
			// WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [%s] to %s", msg, MSG_DUMPER_FACILITY_DESC[i]);
			ret = msg_dumper_thread[i]->write_msg(timep, msg_dumper_severity, msg);
			if (CHECK_FAILURE(ret))
				return ret;
		}
	}

	return ret;
}

unsigned short MsgDumperMgr::deinitialize()
{
	if (!is_init)
	{
		WRITE_ERR_SYSLOG("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper_thread[i] != NULL)
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Release the Object: MsgDumper%s", MSG_DUMPER_FACILITY_DESC[i]);
//			ret = msg_dumper_thread[i]->deinitialize();
			if (CHECK_FAILURE(ret))
				return ret;

			delete msg_dumper_thread[i];
			msg_dumper_thread[i] = NULL;
		}
	}

	is_init = false;

	return MSG_DUMPER_SUCCESS;
}

