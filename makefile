BASE_DIR= ${HOME}/develop/server

DEBUG_LIB_DIR = $(BASE_DIR)/lib/Debug
RELEASE_LIB_DIR = $(BASE_DIR)/lib/Release

ifeq ($(mode),d)
	BIN_SUFFIX = _dbg
	LIB_DIR = $(DEBUG_LIB_DIR)
	BIN_DIR = $(BASE_DIR)/bin/Debug
	CPPFLAGS= -g -gdwarf-2 -fPIC -Wall -DDEBUG $(INC) -Wno-invalid-offsetof
	LDFLAGS = -g -fPIC -L$(LIB_DIR) -lpthread -llogger -lcommon -lframe -lnetevent -lcommon -ltinyxml -lhiredis -lrt -levent
	DEBUG_TARGET = $(BIN_DIR)/chat$(BIN_SUFFIX)
	TARGET	= $(DEBUG_TARGET)
else
	LIB_DIR = $(RELEASE_LIB_DIR)
	BIN_DIR = $(BASE_DIR)/bin/Release
	CPPFLAGS= -g -fPIC -Wall $(INC) -Wno-invalid-offsetof
	LDFLAGS = -g -fPIC -L$(LIB_DIR) -lpthread -llogger -lcommon -lframe -lnetevent -lcommon -ltinyxml -lhiredis -lrt -levent
	RELEASE_TARGET = $(BIN_DIR)/chat$(BIN_SUFFIX)
	TARGET	= $(RELEASE_TARGET)
endif

SERVER_DIR = $(BASE_DIR)/chat
DISPATCH_DIR = dispatch
LOGIC_DIR = logic
BANK_DIR = bank
CONFIG_DIR = config

SRC = $(wildcard *.cpp)
DISPATCH_SRC = $(wildcard $(DISPATCH_DIR)/*.cpp)
LOGIC_SRC = $(wildcard $(LOGIC_DIR)/*.cpp)
BANK_SRC = $(wildcard $(BANK_DIR)/*.cpp)
CONFIG_SRC = $(wildcard $(CONFIG_DIR)/*.cpp)

OBJ_DIR	= $(SERVER_DIR)/.objs
SERVER_OBJS = $(addprefix $(OBJ_DIR)/, $(subst .cpp,.o,$(SRC)))
OBJS = $(wildcard $(OBJ_DIR)/*.o)

COMMON_INCLUDE_DIR = $(BASE_DIR)/common
NETEVENT_INCLUDE_DIR = $(BASE_DIR)/netevent
LOGGER_INCLUDE_DIR = $(BASE_DIR)/logger
FRAME_INCLUDE_DIR = $(BASE_DIR)/frame
REDISCLIENT_INCLUDE_DIR = $(BASE_DIR)/redisclient
INC = -I$(COMMON_INCLUDE_DIR) -I$(NETEVENT_INCLUDE_DIR) -I$(LOGGER_INCLUDE_DIR) -I$(FRAME_INCLUDE_DIR) -I$(REDISCLIENT_INCLUDE_DIR)

all : $(TARGET)

$(TARGET) : $(SERVER_OBJS) DISPATCH LOGIC BANK CONFIG
	$(CXX)  -o $@ $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@


DISPATCH:
	cd $(DISPATCH_DIR); make

LOGIC:
	cd $(LOGIC_DIR); make

BANK:
	cd $(BANK_DIR); make

CONFIG:
	cd $(CONFIG_DIR); make

clean:
	cd $(DISPATCH_DIR); make clean;
	cd $(LOGIC_DIR); make clean;
	cd $(BANK_DIR); make clean;
	cd $(CONFIG_DIR); make clean;
	rm -f $(OBJS) $(TARGET)
