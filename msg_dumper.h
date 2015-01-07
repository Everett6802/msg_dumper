#ifndef MSG_DUMPER_H
#define MSG_DUMPER_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Return values
static const unsigned short MSG_DUMPER_SUCCESS = 0;

static const unsigned short MSG_DUMPER_FAILURE_UNKNOWN = 1;
static const unsigned short MSG_DUMPER_FAILURE_INVALID_ARGUMENT = 2;
static const unsigned short MSG_DUMPER_FAILURE_INVALID_POINTER = 3;
static const unsigned short MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY = 4;
static const unsigned short MSG_DUMPER_FAILURE_OPEN_FILE = 5;
static const unsigned short MSG_DUMPER_FAILURE_NOT_FOUND = 6;
static const unsigned short MSG_DUMPER_FAILURE_INCORRECT_CONFIG = 7;
static const unsigned short MSG_DUMPER_FAILURE_INCORRECT_OPERATION = 8;
static const unsigned short MSG_DUMPER_FAILURE_COM_PORT = 9;
static const unsigned short MSG_DUMPER_FAILURE_MYSQL = 10;
static const unsigned short MSG_DUMPER_FAILURE_SOCKET = 11;

#define CHECK_MSG_DUMPER_FAILURE(x) (x != MSG_DUMPER_SUCCESS ? true : false)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants

static const unsigned short MSG_DUMPER_SEVIRITY_ERROR = 0;
static const unsigned short MSG_DUMPER_SEVIRITY_WARN = 1;
static const unsigned short MSG_DUMPER_SEVIRITY_INFO = 2;
static const unsigned short MSG_DUMPER_SEVIRITY_DEBUG = 3;
static char* MSG_DUMPER_SEVERITY_DESC[] = {"Error", "Warn", "Info", "Debug"};

static const unsigned short MSG_DUMPER_FACILITY_LOG = 0x1;
static const unsigned short MSG_DUMPER_FACILITY_COM = 0x1 << 1;
static const unsigned short MSG_DUMPER_FACILITY_SQL = 0x1 << 2;
static const unsigned short MSG_DUMPER_FACILITY_REMOTE = 0x1 << 3;
static const unsigned short MSG_DUMPER_FACILITY_SYSLOG = 0x1 << 4;
static const unsigned short MSG_DUMPER_FACILITY_ALL = (MSG_DUMPER_FACILITY_LOG | MSG_DUMPER_FACILITY_COM | MSG_DUMPER_FACILITY_SQL | MSG_DUMPER_FACILITY_REMOTE | MSG_DUMPER_FACILITY_SYSLOG);
enum MSG_DUMPER_FACILITY{FACILITY_LOG, FACILITY_COM, FACILITY_SQL, FACILITY_REMOTE, FACILITY_SYSLOG, FACILITY_SIZE};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// APIs

extern "C"
{

unsigned short msg_dumper_get_version(unsigned char& major_version, unsigned char& minor_version);
unsigned short msg_dumper_initialize(const char* config_path);
unsigned short msg_dumper_set_severity(unsigned short severity);
unsigned short msg_dumper_set_severity_ex(unsigned short facility, unsigned short severity);
unsigned short msg_dumper_set_facility(unsigned short facility);
unsigned short msg_dumper_get_severity();
unsigned short msg_dumper_get_severity_ex(unsigned short facility);
unsigned short msg_dumper_get_facility();
unsigned short msg_dumper_write_msg(unsigned short severity, const char* msg);
unsigned short msg_dumper_write_format_msg(unsigned short severity, const char* fmt, ...);
unsigned short msg_dumper_deinitialize();

}

typedef unsigned short (*FP_msg_dumper_get_version)(unsigned char& major_version, unsigned char& minor_version);
typedef unsigned short (*FP_msg_dumper_initialize)(const char* config_path);
typedef unsigned short (*FP_msg_dumper_set_severity)(unsigned short severity);
typedef unsigned short (*FP_msg_dumper_set_severity_ex)(unsigned short facility, unsigned short severity);
typedef unsigned short (*FP_msg_dumper_set_facility)(unsigned short facility);
typedef unsigned short (*FP_msg_dumper_get_severity)();
typedef unsigned short (*FP_msg_dumper_get_severity_ex)(unsigned short facility);
typedef unsigned short (*FP_msg_dumper_get_facility)();
typedef unsigned short (*FP_msg_dumper_write_msg)(unsigned short severity, const char* msg);
typedef unsigned short (*FP_msg_dumper_write_format_msg)(unsigned short severity, const char* fmt, ...);
typedef unsigned short (*FP_msg_dumper_deinitialize)();


#endif
