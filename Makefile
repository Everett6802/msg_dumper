CC := g++

CXXFLAGS := -g -Wall -fPIC
#INCLUDE_PATH := ../include

SOURCES := msg_dumper.cpp msg_dumper_mgr.cpp
OBJS := $(SOURCES:.cpp=.o)

OUTPUT := libmsg_dumper.so

build: $(OUTPUT)
	cp $(OUTPUT) ./test/

$(OUTPUT): $(OBJS)
	$(CC) $(CXXFLAGS) -shared -o $@ $^

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
	rm $(OUTPUT)
	rm ./test/$(OUTPUT)
