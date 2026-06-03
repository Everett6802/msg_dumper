CC := g++

# 0: 不需要 SQL dumper，1: 需要 SQL dumper
NEED_SQL_DUMPER ?= 0
# 0: None，1: syslog，2: console
LOG_BACKEND ?= 0

# c++0x 是 C++11 還在草案階段時的暱稱，正式發布後正確寫法是 c++11 
# codebase是偏底層的系統程式，用 c++14 是比較穩健的選擇，改動最小但又不用卡在 c++0x 這個過時寫法上
CXXFLAGS := -g -Wall -fPIC -std=c++14
CXXFLAGS += -DNEED_SQL_DUMPER=$(NEED_SQL_DUMPER)
CXXFLAGS += -DLOG_BACKEND=$(LOG_BACKEND)
#INCLUDE_PATH := ../include

# Detect Docker or Host
ifeq ($(shell grep -qE '(docker|kubepods)' /proc/1/cgroup 2>/dev/null && echo yes),yes)
    BIN_DIR := /app/bin
    OBJ_DIR := /app/obj
	TEST_DIR := /app/test
else
    BIN_DIR := ./bin
    OBJ_DIR := ./obj
	TEST_DIR := ./test
endif

# SOURCES := common.cpp msg_dumper.cpp msg_dumper_mgr.cpp msg_dumper_base.cpp msg_dumper_timer_thread.cpp msg_dumper_log.cpp msg_dumper_com.cpp msg_dumper_sql.cpp msg_dumper_remote.cpp msg_dumper_syslog.cpp msg_dumper_stdout.cpp
SOURCES := common.cpp msg_dumper.cpp msg_dumper_mgr.cpp msg_dumper_base.cpp msg_dumper_timer_thread.cpp msg_dumper_log.cpp msg_dumper_com.cpp msg_dumper_remote.cpp msg_dumper_syslog.cpp msg_dumper_stdout.cpp
# 條件加入 SQL
ifeq ($(NEED_SQL_DUMPER),1)
SOURCES += msg_dumper_sql.cpp
endif
# OBJS := $(SOURCES:.cpp=.o)
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
# .o 依賴沒有 .d 檔案（include 變動不會 rebuild）
DEPS := $(OBJS:.o=.d)
TEST_SOURCES := common.cpp test_msg_dumper.cpp
# OBJS := $(SOURCES:.cpp=.o)
TEST_OBJS := $(patsubst %.cpp,$(TEST_DIR)/%.o,$(TEST_SOURCES))

LIB_MSG_DUMPER_HEADER := msg_dumper.h
LIB_MSG_DUMPER := libmsg_dumper.so
TEST_MSG_DUMPER := test_msg_dumper

# 預設 libs
LIBS := -lpthread
ifeq ($(NEED_SQL_DUMPER),1)
  LIBS += -L/usr/lib64 -lmysqlclient
endif

CONF_FOLDER := conf
CONF_FILES := $(wildcard $(BIN_DIR)/*)
OUTPUT := $(BIN_DIR)/$(LIB_MSG_DUMPER)
TEST_OUTPUT := $(TEST_DIR)/$(TEST_MSG_DUMPER)

# =========================
# Build
# =========================
build: prepare $(OUTPUT) copy_conf # copy_so

build_test:  $(TEST_OUTPUT)
	cp $(BIN_DIR)/$(LIB_MSG_DUMPER) $(TEST_DIR)
	cp $(LIB_MSG_DUMPER_HEADER) $(TEST_DIR)
# 	@mkdir -p $(TEST_DIR)/$(CONF_FOLDER)
	cp -r $(CONF_FOLDER) $(TEST_DIR)/$(CONF_FOLDER)

build_all: build build_test

prepare:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/conf
	@mkdir -p $(OBJ_DIR)

copy_conf: $(CONF_FILES)
	cp conf/* $(BIN_DIR)/conf/

# copy_so: $(LIB_MSG_DUMPER)
# # # 	cp $(LIB_MSG_DUMPER) $(BIN_DIR)/$(LIB_MSG_DUMPER)

$(OUTPUT): $(OBJS)
	$(CC) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)

# Compile rule
# 在 pattern rule 裡加 mkdir -p 是保險作法 因為Make 的執行順序不保證先跑 prepare
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@
# %.o: %.cpp
# 	$(CC) $(CXXFLAGS) -c -o $@ $<

$(TEST_OUTPUT): $(TEST_OBJS)
# 	$(CC) $(CXXFLAGS) -o $@ $^ $(TEST_LIBS)
	$(CC) $(CXXFLAGS) -o $@ $^ -ldl

# Test Compile rule
$(TEST_DIR)/%.o: %.cpp
	@mkdir -p $(TEST_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@


# =========================
# Clean
# =========================
clean:
	rm -rf $(OBJ_DIR) 2> /dev/null
	rm -rf $(BIN_DIR) 2> /dev/null
	rm -rf $(OUTPUT) 2> /dev/null

clean_test:
	rm -rf $(TEST_DIR) 2> /dev/null

clean_all: clean clean_test
