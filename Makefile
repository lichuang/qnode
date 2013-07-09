DIR=.
BIN_DIR=$(DIR)/bin
SRC_DIR=$(DIR)/src
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/qserver
LUA_DIR=./lua-5.1.4
LUA=$(BIN_DIR)/lua

EXTENSION=c
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE= -I$(INCLUDE_DIR) -I$(LUA_DIR)/src
		
CC=gcc
CFLAGS=-Wall -Werror -g 
#LDFLAGS= -lpthread -rdynamic -llua -ldl -lm -ltcmalloc
LDFLAGS= -L ./lib -lpthread -rdynamic -llua -ldl -lm

all:$(OBJS) $(LUA)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(LUA):
	cd $(LUA_DIR) && make linux
	cp $(LUA_DIR)/src/lua ./bin/
	cp $(LUA_DIR)/src/luac ./bin
	cp $(LUA_DIR)/src/liblua.a ./lib

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

rebuild:
	make clean
	make

clean:
	rm -rf $(OBJS) $(BIN_DIR)/*
