CC := g++

CXXFLAGS := -g -Wall -fPIC -std=c++0x

SOURCES := msg_dumper_wrapper.cpp test_msg_dumper.cpp
OBJS := $(SOURCES:.cpp=.o)
LIB_MSG_DUMPER := libmsg_dumper.so
LIB_MSG_DUMPER_HEADER := msg_dumper.h
LIB_MSG_DUMPER_WRAPPER_FOLDER := wrapper
LIB_MSG_DUMPER_WRAPPER_HEADER := msg_dumper_wrapper.h
LIB_MSG_DUMPER_WRAPPER_SOURCE := msg_dumper_wrapper.cpp

OUTPUT := test_msg_dumper

.PHONY: build
build: $(LIB_MSG_DUMPER_WRAPPER_HEADER) $(LIB_MSG_DUMPER_WRAPPER_SOURCE) $(LIB_MSG_DUMPER_HEADER) $(LIB_MSG_DUMPER) $(OUTPUT)

$(OUTPUT): $(OBJS) $(LIB_MSG_DUMPER)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) -Wl,--start-group $(LIB_MSG_DUMPER) -Wl,--end-group -lrt -ldl

$(LIB_MSG_DUMPER_WRAPPER_HEADER):
	cp ../$(LIB_MSG_DUMPER_WRAPPER_FOLDER)/$(LIB_MSG_DUMPER_WRAPPER_HEADER) .

$(LIB_MSG_DUMPER_WRAPPER_SOURCE):
	cp ../$(LIB_MSG_DUMPER_WRAPPER_FOLDER)/$(LIB_MSG_DUMPER_WRAPPER_SOURCE) .

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS) 2> /dev/null
	rm -f $(OUTPUT) 2> /dev/null
	rm -f $(LIB_MSG_DUMPER_WRAPPER_HEADER) 2> /dev/null
	rm -f $(LIB_MSG_DUMPER_WRAPPER_SOURCE) 2> /dev/null
