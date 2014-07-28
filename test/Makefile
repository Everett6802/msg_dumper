CC := g++

CXXFLAGS := -g -Wall -fPIC

SOURCES := test_msg_dumper.cpp
OBJS := $(SOURCES:.cpp=.o)
LIBS := libmsg_dumper.so

OUTPUT := test_msg_dumper

build: $(OUTPUT)

$(OUTPUT): $(OBJS) $(LIBS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) -Wl,--start-group $(LIBS) -Wl,--end-group -lrt -ldl

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
	rm $(OUTPUT)
	rm ./test/$(OUTPUT)
