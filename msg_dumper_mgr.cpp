#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"
#if NEED_SQL_DUMPER == NEED_SQL_DUMPER_YES
#include "msg_dumper_sql.h"
#endif
#include "msg_dumper_remote.h"
#include "msg_dumper_syslog.h"
#include "msg_dumper_stdout.h"


using namespace std;

//int MsgDumperMgr::facility_name_size = sizeof(MSG_DUMPER_FACILITY_DESC) / sizeof(MSG_DUMPER_FACILITY_DESC[0]);
#if NEED_SQL_DUMPER == NEED_SQL_DUMPER_YES
short MsgDumperMgr::FACILITY_FLAGS[] = {MSG_DUMPER_FACILITY_FLAG_LOG, MSG_DUMPER_FACILITY_FLAG_COM, MSG_DUMPER_FACILITY_FLAG_SQL, MSG_DUMPER_FACILITY_FLAG_REMOTE, MSG_DUMPER_FACILITY_FLAG_SYSLOG, MSG_DUMPER_FACILITY_FLAG_STDOUT};
#else
short MsgDumperMgr::FACILITY_FLAGS[] = {MSG_DUMPER_FACILITY_FLAG_LOG, MSG_DUMPER_FACILITY_FLAG_COM, MSG_DUMPER_FACILITY_FLAG_REMOTE, MSG_DUMPER_FACILITY_FLAG_SYSLOG, MSG_DUMPER_FACILITY_FLAG_STDOUT};
#endif
int MsgDumperMgr::FACILITY_FLAG_SIZE = sizeof(FACILITY_FLAGS) / sizeof(FACILITY_FLAGS[0]);

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
		// fprintf(stderr, "MsgDumperFacilityFactory::construct() called, n: %s\n", n.c_str());
		// map_type::iterator iter = m_classes.begin();
		// while (iter != m_classes.end())
		// {
		// 	fprintf(stderr, "Registered class: %s\n", iter->first.c_str());
		// 	iter++;
		// }
		map_type::iterator i = m_classes.find(n);
		if (i == m_classes.end())
		{
			WRITE_ERROR("Fail to find the %s class", n.c_str());
			throw invalid_argument("Unknown class");
			return NULL; // or throw or whatever you want
		}

		MsgDumperBase* msg_dumper = (MsgDumperBase*)i->second();  // Allocate the memory of a specific type
		if (msg_dumper == NULL)
		{
			WRITE_ERROR("Fail to allocate the MsgDumper%s object", n.c_str());
			throw bad_alloc();
		}
		return msg_dumper;
	}

	int register_class_size()const{return m_classes.size();}
};

MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_facility_flag(MSG_DUMPER_FACILITY_FLAG_NONE)
{
	facility_factory = new MsgDumperFacilityFactory();
	if (facility_factory == NULL)
		throw bad_alloc();

// Register the class to the simple factory
	REGISTER_CLASS(MsgDumperLog);
	REGISTER_CLASS(MsgDumperCom);
#if NEED_SQL_DUMPER == NEED_SQL_DUMPER_YES
	REGISTER_CLASS(MsgDumperSql);
#endif
	REGISTER_CLASS(MsgDumperRemote);
	REGISTER_CLASS(MsgDumperSyslog);
	REGISTER_CLASS(MsgDumperStdout);
// Check the parameter setting is correct
	// fprintf(stderr, "MSG_DUMPER_FACILITY_DESC_LEN: %d, FACILITY_SIZE: %d, FACILITY_FLAG_SIZE: %d\n", MSG_DUMPER_FACILITY_DESC_LEN, FACILITY_SIZE, FACILITY_FLAG_SIZE);
	assert((facility_factory->register_class_size() == FACILITY_SIZE) && "The facility size is NOT identical");
	assert((MSG_DUMPER_FACILITY_DESC_LEN == FACILITY_SIZE) && "The facility name size is NOT identical");
	assert((FACILITY_FLAG_SIZE == FACILITY_SIZE) && "The facility flag size is NOT identical");

	memset(current_working_directory, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		dumper_severity_arr[i] = MSG_DUMPER_SEVERITY_ERROR;

// Create the facility mapping table
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		facility_mapping_table.insert(make_pair(FACILITY_FLAGS[i], (MSG_DUMPER_FACILITY)i));

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

// int MsgDumperMgr::get_facility_index(unsigned short facility_flag)const
// {
// 	if (!(MSG_DUMPER_FACILITY_ALL & facility_flag))
// 	{
// 		WRITE_ERROR("Incorrect facility flag: %d", facility_flag);
// 		throw invalid_argument("Incorrect facility flag");
// 	}
// 	return (int)facility_mapping_table.find(facility_flag)->second;
// }

// unsigned short MsgDumperMgr::set_severity(unsigned short msg_dumper_severity, unsigned short facility)
// {
// 	// if (is_init)
// 	// {
// 	// 	WRITE_ERROR("Library has been initialized");
// 	// 	return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
// 	// }
// 	int facility_index = (int)get_facility_index(facility);
// 	WRITE_DEBUG("Set the severity[%s] to facility[%s]", MSG_DUMPER_SEVERITY_DESC[msg_dumper_severity], MSG_DUMPER_FACILITY_DESC[facility_index]);
// 	dumper_severity_arr[facility_index] = msg_dumper_severity;

// 	return MSG_DUMPER_SUCCESS;
// }

// unsigned short MsgDumperMgr::set_severity_all(unsigned short msg_dumper_severity)
// {
// 	// if (is_init)
// 	// {
// 	// 	WRITE_ERROR("Library has been initialized");
// 	// 	return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
// 	// }
// 	WRITE_DEBUG("Set the severity[%d] to all facilities", msg_dumper_severity);
// 	for (int i = 0 ; i < FACILITY_SIZE ; i++)
// 		dumper_severity_arr[i] = msg_dumper_severity;

// 	return MSG_DUMPER_SUCCESS;
// }

// unsigned short MsgDumperMgr::set_facility(unsigned short facility)
// {
// 	// if (is_init)
// 	// {
// 	// 	WRITE_ERROR("Library has been initialized");
// 	// 	return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
// 	// }
// 	WRITE_DEBUG("Set the facility to %d", facility);
// 	dumper_facility_flag = facility;

// 	return MSG_DUMPER_SUCCESS;
// }

// unsigned short MsgDumperMgr::get_severity(unsigned short facility)const
// {
// 	int facility_index = (int)get_facility_index(facility);
// 	unsigned short msg_dumper_severity = dumper_severity_arr[facility_index];
// 	WRITE_DEBUG("Get the severity of facility[%s]: %d", MSG_DUMPER_FACILITY_DESC[facility_index], msg_dumper_severity);
// 	return msg_dumper_severity;
// }

// unsigned short MsgDumperMgr::get_facility()const
// {
// 	return dumper_facility_flag;
// }

unsigned short MsgDumperMgr::set_severity(unsigned short facility_index, unsigned short severity_index)
{
	if (is_init)
	{
		WRITE_ERROR("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	if (facility_index < 0 || facility_index >= FACILITY_SIZE)
	{
		WRITE_ERROR("Incorrect facility index: %d", facility_index);
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	if (severity_index < 0 || severity_index >= MSG_DUMPER_SEVERITY_LIST_SIZE)
	{
		WRITE_ERROR("Incorrect severity index: %d", severity_index);
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	dumper_facility_flag |= FACILITY_FLAGS[facility_index];
	dumper_severity_arr[facility_index] = severity_index;
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::get_severity(unsigned short facility_index, unsigned short& severity_index)const
{
	if (facility_index < 0 || facility_index >= FACILITY_SIZE)
	{
		WRITE_ERROR("Incorrect facility index: %d", facility_index);
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}
	if (dumper_facility_flag & FACILITY_FLAGS[facility_index])
		severity_index = dumper_severity_arr[facility_index];
	else
		severity_index = MSG_DUMPER_SEVERITY_NOSET;
	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::initialize()
{
	if (is_init)
	{
		WRITE_ERROR("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL)
	{
		WRITE_DEBUG("Fail to get the current working folder");
		return MSG_DUMPER_FAILURE_UNKNOWN;
	}
	WRITE_DEBUG("Current working folder: %s", current_working_directory);

	unsigned short ret = MSG_DUMPER_SUCCESS;
	char facility_class_name[32];
// Initialize the device
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		snprintf(facility_class_name, 32, "MsgDumper%s", MSG_DUMPER_FACILITY_DESC[i]);
// Allocate and initialize the object derived from the MsgDumplerBase class
		if (dumper_facility_flag & FACILITY_FLAGS[i])
		{
			WRITE_DEBUG("Allocate the %s object", facility_class_name);
			msg_dumper_thread[i] = new MsgDumperTimerThread(facility_factory->construct(facility_class_name));
			if (msg_dumper_thread[i] == NULL)
			{
				WRITE_ERROR("Fail to allocate the %s object", facility_class_name);
				return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
			}
			WRITE_DEBUG("Initialize the %s object", facility_class_name);
			ret = msg_dumper_thread[i]->initialize(current_working_directory);
			if (CHECK_FAILURE(ret))
			{
				WRITE_ERROR("Fail to initialize the %s object", facility_class_name);
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
				WRITE_DEBUG("Deinitialize the MsgDumper%s object due to the failure of initialization of some objects", MSG_DUMPER_FACILITY_DESC[i]);
				msg_dumper_thread[i]->deinitialize();
				delete msg_dumper_thread[i];
				msg_dumper_thread[i] = NULL;
			}
		}
	}
	return ret;
}

unsigned short MsgDumperMgr::write_msg(unsigned short msg_dumper_severity, const char* msg)
{
	if (!is_init)
	{
		WRITE_ERROR("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}
	if (msg == NULL)
	{
		WRITE_ERROR("Invalid pointer: msg");
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
// Mark the time that the message is generated
	time_t timep;
	time(&timep);
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		// fprintf(stderr, "MsgDumperMgr::write_msg [msg_dumper_severity: %d, message: %s]\n", msg_dumper_severity, msg);
		if (msg_dumper_thread[i] && msg_dumper_severity <= dumper_severity_arr[i])
		{
			// WRITE_DEBUG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [%s] to %s", msg, MSG_DUMPER_FACILITY_DESC[i]);
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
		WRITE_ERROR("Library is Not initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	unsigned short ret = MSG_DUMPER_SUCCESS;
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper_thread[i] != NULL)
		{
			WRITE_DEBUG("Release the Object: MsgDumper%s", MSG_DUMPER_FACILITY_DESC[i]);
			ret = msg_dumper_thread[i]->deinitialize();
			if (CHECK_FAILURE(ret))
				return ret;
			delete msg_dumper_thread[i];
			msg_dumper_thread[i] = NULL;
		}
	}
	is_init = false;
	return MSG_DUMPER_SUCCESS;
}

