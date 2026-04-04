#include <stdexcept>
#include "common.h"


using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
const char* MODULE_NAME = "MSG_DUMPER";
const unsigned char MAJOR_VERSION = 1;
const unsigned char MINOR_VERSION = 4;
const unsigned char BUILD_VERSION = 0;
const char* MSG_DUMPER_ERROR_COLOR = "\x1B[31m";
const char* MSG_DUMPER_SEVERITY_DESC[] = {"ERROR", "WARN", "INFO", "DEBUG"};
const int MSG_DUMPER_SEVERITY_DESC_LEN = sizeof(MSG_DUMPER_SEVERITY_DESC) / sizeof(MSG_DUMPER_SEVERITY_DESC[0]);
const char* MSG_DUMPER_SEVERITY_DESC_NOSET = "NOSET";
#if NEED_SQL_DUMPER == NEED_SQL_DUMPER_YES
const char* MSG_DUMPER_FACILITY_DESC[] = {"LOG", "COM", "SQL", "REMOTE", "SYSLOG", "STDOUT"};
#else
const char* MSG_DUMPER_FACILITY_DESC[] = {"LOG", "COM", "REMOTE", "SYSLOG", "STDOUT"};
#endif
const int MSG_DUMPER_FACILITY_DESC_LEN = sizeof(MSG_DUMPER_FACILITY_DESC) / sizeof(MSG_DUMPER_FACILITY_DESC[0]);

//const unsigned short MSG_DUMPER_SHORT_STRING_SIZE = DEF_MSG_DUMPER_SHORT_STRING_SIZE;
//const unsigned short MSG_DUMPER_STRING_SIZE = DEF_MSG_DUMPER_STRING_SIZE;
//const unsigned short MSG_DUMPER_LONG_STRING_SIZE = DEF_MSG_DUMPER_LONG_STRING_SIZE;
//const unsigned short MSG_DUMPER_EX_LONG_STRING_SIZE = DEF_MSG_DUMPER_LONG_STRING_SIZE * 2;

const unsigned short SERVER_PORT_NO = 6802;
const unsigned short MAX_CONNECTED_CLIENT = 5;

const char* error_description[] =
{
	"Success",
	"Failure Unknown",
	"Failure Invalid Argument",
	"Failure Invalid Pointer",
	"Failure Insufficient Memory",
	"Failure System API",
	"Failure Not Found",
	"Failure Out of Range",
	"Failure Incorrect Config",
	"Failure Incorrect Operation",
	"Failure COM Port",
	"Failure MySQL",
	"Failure Socket",
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
unsigned short transform_linux_severity_to_msg_dumper_severity(unsigned short linux_severity)
{
#if 0
# linux severity
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#endif
	unsigned short msg_dumper_severity;
	switch(linux_severity)
	{
	case LOG_DEBUG:
		msg_dumper_severity = MSG_DUMPER_SEVERITY_DEBUG;
		break;
	case LOG_INFO:
		msg_dumper_severity = MSG_DUMPER_SEVERITY_INFO;
		break;
	case LOG_WARNING:
	case LOG_NOTICE:
		msg_dumper_severity = MSG_DUMPER_SEVERITY_WARN;
		break;
	case LOG_EMERG:
	case LOG_ALERT:
	case LOG_CRIT:
	case LOG_ERR:
		msg_dumper_severity = MSG_DUMPER_SEVERITY_ERROR;
		break;
	default:
		{
			fprintf(stderr, "%sUnknown linux severity: %d\n", MSG_DUMPER_ERROR_COLOR, linux_severity);
			throw invalid_argument("Incorrect linux severity");
		}
		break;
	}
	// fprintf(stderr, "transform_linux_severity_to_msg_dumper_severity linux_severity: %d, msg_dumper_severity: %d\n", linux_severity, msg_dumper_severity);
	return msg_dumper_severity;
}

unsigned short transform_msg_dumper_severity_to_linux_severity(unsigned short msg_dumper_severity)
{
#if 0
static const unsigned short MSG_DUMPER_SEVERITY_ERROR = 0;
static const unsigned short MSG_DUMPER_SEVERITY_WARN = 1;
static const unsigned short MSG_DUMPER_SEVERITY_INFO = 2;
static const unsigned short MSG_DUMPER_SEVERITY_DEBUG = 3;
#endif
	unsigned short linux_severity;
	switch(msg_dumper_severity)
	{
	case MSG_DUMPER_SEVERITY_DEBUG:
		linux_severity = LOG_DEBUG;
		break;
	case MSG_DUMPER_SEVERITY_INFO:
		linux_severity = LOG_INFO;
		break;
	case MSG_DUMPER_SEVERITY_WARN:
		linux_severity = LOG_WARNING;
		break;
	case MSG_DUMPER_SEVERITY_ERROR:
		linux_severity = LOG_ERR;
		break;
	default:
		{
			fprintf(stderr, "%sUnknown msg dumper severity: %d\n", MSG_DUMPER_ERROR_COLOR, msg_dumper_severity);
			throw invalid_argument("Incorrect msg dumper severity");
		}
		break;
	}
	return linux_severity;
}

void convert_str2upper(char *str)
{
    int len = strlen(str);
    while (len--) 
	{
        *str = toupper(*str);
        str++;
    }
}

void convert_str2lower(char *str)
{
    int len = strlen(str);
    while (len--) 
	{
        *str = tolower(*str);
        str++;
    }
}

unsigned short safe_snprintf(char **out_buf, size_t init_size, const char *fmt, ...)
{
	if (out_buf == NULL || fmt == NULL || init_size == 0)
		return MSG_DUMPER_FAILURE_INVALID_ARGUMENT;
    va_list args;
    va_start(args, fmt);
    char *buf = (char*)malloc(init_size);
    if (buf == NULL) 
	{
        va_end(args);
        return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
    }
    int ret = vsnprintf(buf, init_size, fmt, args);
    va_end(args);
    if (ret < 0) 
	{
        free(buf);
        return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
    }
	else if ((size_t)ret >= init_size) 
	{
        size_t needed = ret + 1;
        char *new_buf = (char*)realloc(buf, needed);
        if (!new_buf) 
		{
            free(buf);
            return MSG_DUMPER_FAILURE_INSUFFICIENT_MEMORY;
        }
        va_start(args, fmt);
        vsnprintf(new_buf, needed, fmt, args);
        va_end(args);
        buf = new_buf;
    }
    *out_buf = buf;
    return MSG_DUMPER_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class

MsgCfg::MsgCfg(const time_t& new_timep, unsigned short new_msg_dumper_severity, const char* new_data)
{
	timep = new_timep;
	p = localtime(&timep);
	snprintf(date_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d/%02d/%02d", (1900 + p->tm_year) % 2000, p->tm_mon + 1, p->tm_mday);
	snprintf(time_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);

	msg_dumper_severity = new_msg_dumper_severity;
	memset(data, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
	snprintf(data, MSG_DUMPER_LONG_STRING_SIZE, "%s", new_data);

	format_message = NULL;
}

const char* MsgCfg::to_string()
{
	if (format_message == NULL)
	{
		int date_str_len = strlen(date_str);
		int time_str_len = strlen(time_str);
		int severity_desc_len = strlen(MSG_DUMPER_SEVERITY_DESC[msg_dumper_severity]);
		int title_len = date_str_len + time_str_len + severity_desc_len + 6;
		format_message = new char[title_len + MSG_DUMPER_LONG_STRING_SIZE];
		if (format_message == NULL)
		{
			assert(0 && "Fail to allocate the format_message");
			return NULL;
		}
		snprintf(format_message, MSG_DUMPER_LONG_STRING_SIZE, "[%s %s %s] %s\n", date_str, time_str, MSG_DUMPER_SEVERITY_DESC[msg_dumper_severity], data);
	}
	return format_message;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ScopedCStr::ScopedCStr() : 
	cstr(NULL) 
{

}

ScopedCStr::~ScopedCStr()
{
	if (cstr != NULL)
	{
        free(cstr);
        cstr = NULL;
    }
}

char* ScopedCStr::release()
{
	char *tmp = NULL;
	if (cstr != NULL)
	{
		tmp = cstr;
		cstr = NULL;
	}
	return tmp;
}

