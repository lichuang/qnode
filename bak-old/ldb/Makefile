DIR=.
BIN_DIR=$(DIR)/bin
SRC_DIR=$(DIR)/src
LUA_DIR=$(shell cd ../lua-5.1.4 && pwd)
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
LIB_DIR=$(DIR)/lib
LIB=libldb.a

EXTENSION=c
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE= -I$(INCLUDE_DIR) -I$(LUA_DIR)/src
		
CC=gcc
AR= ar rcu
CFLAGS=-Wall -Werror -g 
LDFLAGS= -L ./lib -L ../lib -rdynamic -llua -ldl -lm -lldb -lreadline

all:$(OBJS)
	$(AR) $(LIB_DIR)/$(LIB) $(OBJS)
	$(CC) test/main.c -I./src $(INCLUDE) -o test/test $(LDFLAGS)

test:$(LIB_DIR)/$(LIB)
	$(CC) test/main.c -I./src $(INCLUDE) -o test/test $(LDFLAGS)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

rebuild:
	make clean
	make

clean:
	rm -rf $(OBJS) $(BIN_DIR)/* $(LIB_DIR)/*
