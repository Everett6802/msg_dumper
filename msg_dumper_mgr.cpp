#include "msg_dumper.h"
#include "common.h"
#include "msg_dumper_mgr.h"
#include "msg_dumper_log.h"
#include "msg_dumper_com.h"
#include "msg_dumper_sql.h"
#include "msg_dumper_remote.h"
#include "msg_dumper_syslog.h"


char* MsgDumperMgr::dev_name[] = {"Log", "Com", "Sql", "Remote", "Syslog"};
int MsgDumperMgr::dev_name_size = sizeof(dev_name) / sizeof(dev_name[0]);
short MsgDumperMgr::dev_flag[] = {MSG_DUMPER_FACILITY_LOG, MSG_DUMPER_FACILITY_COM, MSG_DUMPER_FACILITY_SQL, MSG_DUMPER_FACILITY_REMOTE, MSG_DUMPER_FACILITY_SYSLOG};
int MsgDumperMgr::dev_flag_size = sizeof(dev_flag) / sizeof(dev_flag[0]);

MsgDumperMgr::MsgDumperMgr() :
	is_init(false),
	dumper_facility(MSG_DUMPER_FACILITY_LOG)
{
// Register the class to the simple factory
	REGISTER_CLASS(MsgDumperLog);
	REGISTER_CLASS(MsgDumperCom);
	REGISTER_CLASS(MsgDumperSql);
	REGISTER_CLASS(MsgDumperRemote);
	REGISTER_CLASS(MsgDumperSyslog);
// Check the parameter setting is correct
	assert((device_factory.register_class_size() == FACILITY_SIZE) && "The facility size is NOT identical");
	assert((dev_name_size == FACILITY_SIZE) && "The facility name size is NOT identical");
	assert((dev_flag_size == FACILITY_SIZE) && "The facility flag size is NOT identical");

	memset(current_working_directory, 0x0, sizeof(char) * MSG_DUMPER_STRING_SIZE);

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		dumper_severity_arr[i] = MSG_DUMPER_SEVIRITY_ERROR;

// Create the facility mapping table
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		facility_mapping_table.insert(make_pair(dev_flag[i], (MSG_DUMPER_FACILITY)i));

	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		msg_dumper[i] = NULL;
}

MsgDumperMgr::~MsgDumperMgr()
{
}


bool MsgDumperMgr::can_ignore(unsigned short severity)const
{
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		if (msg_dumper[i] && severity <= dumper_severity_arr[i])
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
	char dev_class_name[32];
// Initialize the device
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
	{
		snprintf(dev_class_name, 32, "MsgDumper%s", dev_name[i]);
// Allocate and initialize the object derived from the MsgDumplerBase class
		if (dumper_facility & dev_flag[i])
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Allocate the %s object", dev_class_name);
			msg_dumper[i] = device_factory.construct(dev_class_name);
			if (msg_dumper[i] == NULL)
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to allocate the %s object", dev_class_name);
				return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
			}

			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Initialize the %s object", dev_class_name);
			ret = msg_dumper[i]->initialize(current_working_directory);
			if (CHECK_MSG_DUMPER_FAILURE(ret))
			{
				WRITE_ERR_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Fail to initialize the %s object", dev_class_name);
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
			if (msg_dumper[i] != NULL)
			{
				WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Deinitialize the MsgDumper%s object due to the failure of initialization of some objects", dev_name[i]);
				msg_dumper[i]->deinitialize();
				delete msg_dumper[i];
				msg_dumper[i] = NULL;
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

unsigned short MsgDumperMgr::set_severity(unsigned short severity, unsigned short single_facility)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	int facility_index = (int)get_facility_index(single_facility);
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity[%d] to facility[%s]", severity, dev_name[facility_index]);
	dumper_severity_arr[facility_index] = severity;

	return MSG_DUMPER_SUCCESS;
}

unsigned short MsgDumperMgr::set_severity_all(unsigned short severity)
{
	if (is_init)
	{
		WRITE_ERR_SYSLOG("Library has been initialized");
		return MSG_DUMPER_FAILURE_INCORRECT_OPERATION;
	}

	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Set the severity[%d] to all facilities", severity);
	for (int i = 0 ; i < FACILITY_SIZE ; i++)
		dumper_severity_arr[i] = severity;

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
	unsigned short severity = dumper_severity_arr[facility_index];
	WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Get the severity of facility[%s]: %d", dev_name[facility_index], severity);
	return severity;
}

unsigned short MsgDumperMgr::get_facility()const
{
	return dumper_facility;
}

unsigned short MsgDumperMgr::write_msg(unsigned short severity, const char* msg)
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
		if (msg_dumper[i] && severity <= dumper_severity_arr[i])
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_LONG_STRING_SIZE, "Write message [%s] to %s", msg, dev_name[i]);
			ret = msg_dumper[i]->write_msg(timep, severity, msg);
			if (CHECK_MSG_DUMPER_FAILURE(ret))
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
		if (msg_dumper[i] != NULL)
		{
			WRITE_DEBUG_FORMAT_SYSLOG(MSG_DUMPER_STRING_SIZE, "Release the Object: MsgDumper%s", dev_name[i]);
//			ret = msg_dumper[i]->deinitialize();
			if (CHECK_MSG_DUMPER_FAILURE(ret))
				return ret;

			delete msg_dumper[i];
			msg_dumper[i] = NULL;
		}
	}

	is_init = false;

	return MSG_DUMPER_SUCCESS;
}

