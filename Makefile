DIR=.
BIN_DIR=$(DIR)/bin
LIB_DIR=$(DIR)/lib
SRC_DIR=$(DIR)/src
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/qserver
LUA_DIR=./lua-5.1.4
LDB_DIR=./ldb
LUA=$(BIN_DIR)/lua
LDB=$(LIB_DIR)/libldb.a

EXTENSION=c
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE= -I$(INCLUDE_DIR) -I$(LUA_DIR)/src  -I$(LDB_DIR)/src
		
CC=gcc
CFLAGS=-Wall -Werror -g -DDEBUG
#LDFLAGS= -lpthread -rdynamic -llua -ldl -lm -ltcmalloc
LDFLAGS= -L ./lib -lpthread -rdynamic -llua -ldl -lm -lldb -lreadline

all:$(LUA) $(LDB) $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(LUA):
	cd $(LUA_DIR) && make linux
	cp $(LUA_DIR)/src/lua ./bin/
	cp $(LUA_DIR)/src/luac ./bin
	cp $(LUA_DIR)/src/liblua.a $(LIB_DIR)/

$(LDB):
	cd $(LDB_DIR) && make
	cp $(LDB_DIR)/lib/libldb.a $(LIB_DIR)/

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

rebuild:
	make clean
	make

clean:
	rm -rf $(OBJS) $(BIN_DIR)/*
