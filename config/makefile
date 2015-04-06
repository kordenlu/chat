BASE_DIR= ${HOME}/develop/server
SERVER_DIR = $(BASE_DIR)/chat
SERVER_INCLUDE_DIR = $(SERVER_DIR)
SERVER_OBJ_DIR = $(SERVER_DIR)/.objs

SRC = $(wildcard *.cpp)
OBJS = $(addprefix $(SERVER_OBJ_DIR)/, $(subst .cpp,.o,$(SRC)))

CPPFLAGS= -g -gdwarf-2 -fPIC -Wall -DDEBUG $(INC) -Wno-invalid-offsetof

all : $(OBJS)

$(SERVER_OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)