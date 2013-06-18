DIR=.
BIN_DIR=$(DIR)/bin
SRC_DIR=$(DIR)/src
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/qserver

EXTENSION=c
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE= -I$(INCLUDE_DIR)
		
CC=gcc
CFLAGS=-Wall -W -Werror -g 
#LDFLAGS= -lpthread -rdynamic -llua -ldl -lm -ltcmalloc
LDFLAGS= -lpthread -rdynamic -llua -ldl -lm

all:$(OBJS) 
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

rebuild:
	make clean
	make

clean:
	rm -rf $(OBJS) $(BIN_DIR)/*
