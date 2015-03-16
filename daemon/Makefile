CC := g++

CXXFLAGS := -g -Wall -fPIC

SOURCES := msg_dumper_daemon.cpp
OBJS := $(SOURCES:.cpp=.o)

OUTPUT := msg_dumper_daemon

build: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) -lpthread

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) 2> /dev/null
	rm -f $(OUTPUT) 2> /dev/null

