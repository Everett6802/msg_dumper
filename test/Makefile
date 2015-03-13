CC := g++

CXXFLAGS := -g -Wall -fPIC

SOURCES := test_msg_dumper.cpp
OBJS := $(SOURCES:.cpp=.o)
LIBS := libmsg_dumper.so
CONFS := $(PWD)/../conf

OUTPUT := test_msg_dumper

.PHONY: build
build: $(OUTPUT)
	cp -r $(CONFS) .

$(OUTPUT): $(OBJS) $(LIBS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) -Wl,--start-group $(LIBS) -Wl,--end-group -lrt -ldl

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS) 2> /dev/null
	rm -f $(OUTPUT) 2> /dev/null

