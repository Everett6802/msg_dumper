#include "common.h"


MsgCfg::MsgCfg(const time_t& new_timep, unsigned short new_severity, const char* new_data)
{
	timep = new_timep;
		p = localtime(&timep);
		snprintf(date_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d/%02d/%02d", (1900 + p->tm_year) % 2000, p->tm_mon + 1, p->tm_mday);
		snprintf(time_str, MSG_DUMPER_SHORT_STRING_SIZE, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);

		severity = new_severity;
		memset(data, 0x0, sizeof(char) * MSG_DUMPER_LONG_STRING_SIZE);
		snprintf(data, MSG_DUMPER_LONG_STRING_SIZE, "%s", new_data);

		format_message = NULL;
}


const char* MsgCfg::to_string()
{
		if (format_message == NULL)
		{
			format_message = new char[MSG_DUMPER_LONG_STRING_SIZE];
			if (format_message == NULL)
			{
				assert(0 && "Fail to allocate the format_message");
				return NULL;
			}
			snprintf(format_message, MSG_DUMPER_LONG_STRING_SIZE, "[%s %s %s] %s\n", date_str, time_str, MSG_DUMPER_SEVERITY_DESC[severity], data);
		}
		return format_message;
}
