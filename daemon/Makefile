CC := g++

CXXFLAGS := -g -Wall -fPIC

SOURCES := msg_dumper_daemon.cpp
OBJS := $(SOURCES:.cpp=.o)

OUTPUT := msg_dumper_daemon

build: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
	rm $(OUTPUT)

