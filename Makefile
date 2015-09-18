CC := g++

CXXFLAGS := -g -Wall -fPIC -std=c++0x
#INCLUDE_PATH := ../include

SOURCES := common.cpp msg_dumper.cpp msg_dumper_mgr.cpp msg_dumper_base.cpp msg_dumper_timer_thread.cpp msg_dumper_log.cpp msg_dumper_com.cpp msg_dumper_sql.cpp msg_dumper_remote.cpp msg_dumper_syslog.cpp
OBJS := $(SOURCES:.cpp=.o)
LIB_MSG_DUMPER_HEADER := msg_dumper.h
CONF_FOLDER := conf
TEST_FOLDER := test

OUTPUT := libmsg_dumper.so

build: $(OUTPUT)
	cp $(OUTPUT) $(TEST_FOLDER)
	cp $(LIB_MSG_DUMPER_HEADER) $(TEST_FOLDER)
	cp -r $(CONF_FOLDER) $(TEST_FOLDER)

$(OUTPUT): $(OBJS)
	$(CC) $(CXXFLAGS) -shared -o $@ $^ -lpthread -L/usr/lib64 -lmysqlclient

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) 2> /dev/null
	rm -f $(OUTPUT) 2> /dev/null
	rm -f $(TEST_FOLDER)/$(OUTPUT) 2> /dev/null
	rm -f $(TEST_FOLDER)/$(LIB_MSG_DUMPER_HEADER) 2> /dev/null
	rm -rf $(TEST_FOLDER)/$(CONF_FOLDER) 2> /dev/null
