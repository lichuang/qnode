################################################################################
# THIS IS [Makefile_For_GNU_C_CXX_depstar_release] 2014/12/15 (version 0.583.2)
#
# Generic Makefile for C/C++ Program
#
# License: GPL (General Public License)
# Author:  whyglinux <whyglinux AT gmail DOT com>
# Date:    2006/03/04 (version 0.1)
#          2007/03/24 (version 0.2)
#          2007/04/09 (version 0.3)
#          2007/06/26 (version 0.4)
#          2008/04/05 (version 0.5)
# Description:
# ------------
# This is an easily customizable makefile template. The purpose is to
# provide an instant building environment for C/C++ programs.
#
# It searches all the C/C++ source files in the specified directories,
# makes dependencies, compiles and links to form an executable.
#
# Besides its default ability to build C/C++ programs which use only
# standard C/C++ libraries, you can customize the Makefile to build
# those using other libraries. Once done, without any changes you can
# then build programs using the same or less libraries, even if source
# files are renamed, added or removed. Therefore, it is particularly
# convenient to use it to build codes for experimental or study use.
#
# GNU make is expected to use the Makefile. Other versions of makes
# may or may not work.
#
# Usage:
# ------
# 1. Copy the Makefile to your program directory.
# 2. Customize in the "Customizable Section" only if necessary:
#    * to use non-standard C/C++ libraries, set pre-processor or compiler
#      options to <MY_CFLAGS> and linker ones to <MY_LIBS>
#      (See Makefile.gtk+-2.0 for an example)
#    * to search sources in more directories, set to <SRCDIRS>
#    * to specify your favorite program name, set to <PROGRAM>
# 3. Type make to start building your program.
#
# Make Target:
# ------------
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make NODEP=yes compile and link without generating dependencies
#   $ make objs      compile only (no linking)
#   $ make tags      create tags for Emacs editor
#   $ make ctags     create ctags for VI editor
#   $ make clean     clean objects and the executable file
#   $ make distclean clean objects, the executable and dependencies
#   $ make help      get the usage of the makefile
#
#===============================================================================
#
# [Makefile_For_GNU_C_CXX_depstar_release] Description:
# -----------------------------------------
# ken renew it form china on the base of the original: 2008/04/05 (version 0.5).
# License: GPL (General Public License)
# Author:  ken <ken8341 DOT blog DOT 163 DOT com>
#
# ADD Make Target:
# ----------------
# 1. $ make cleanall   Add this function to clean the objects, executable, 
#    dependencies and any files that you can define.
# 2. Support multiple directories for GNU C/C++ program. 
# 3. Support setup SRCs and HEADERs in different directories.
# 4. Support enble/disable auto run the executable program after "make".
# 5. Support display the include directories of the header in "INCDIRS" column.
#
# This is a powerful generic GNU C/C++  "make" template that I can see before.
# Why the modern good idea and technology are not source from china? I Think
# that the main reason is a rubbish animal ideologist of Kong Qiu which born 
# in china, and make china fall behind the world thousands years. Shit confucius
# this hired thugs of the devil. Long live the freedom spirit!
#
# Bug:
# ----
# 1. In C/CXX mix source code work in the Cygwin on windows xp, Seldom when the 
# source code has an special error can cause the Makefile unvalid, Such as 
# "*** missing separator. Stop."error, This show that the error in the "xyz.d" 
# file.
# Solution: 
# Correct error of the source code relation to the name of "xyz", such as 
# "xyz.c" or "xyz.cxx" or "xyz.h" file, etc. 
# Temporary solution is make the error relation source files disable and delete 
# all the “*.d” files by manual can make the Makefile valid again. Another 
# solution perhas can use "$ make NODEP=yes" to make without generating 
# dependencies.
# Reason:
# Not know yet, Once I only move the code to other place in the header file that
# can solve, But when I move the code back to the original place, The error not 
# show again. It is seem that perhas cause by the platform of WinXP and Cygwin.
#///////////////////////////////////////////////////////////////////////////////

## Customizable Section: adapt those variables to suit your program.
##==============================================================================

# The pre-processor and compiler options.
# MY_CFLAGS = -ggdb3 -pipe -O2 -Wall -Wextra -fopenmp -march=native -mfpmath=sse -DLINUX -m64 -std=c++0x
MY_CFLAGS = -g -O0 -Wall -DLINUX

# The linker options.
# MY_LIBS   = -lGLEW -lglut -lGLU -lGL -lX11 -lXmu -lXi -lm -L/usr/X11R6/lib -lgomp -lOpenThreads -lpthread
MY_LIBS   = -lm -lpthread ./lib/install/lua/lib/liblua.a -ldl

# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS  = 

# The options used in linking as well as in any direct use of ld.
LDFLAGS   = 

## The C/C++ header file directories. --ken
# Include options, only need directories, do not need "-I".
INCDIRS = ./src/ ./lib/install/lua/include

# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS   = ./src/base\
            ./src/core\
            ./src/script\
            ./src

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAM   = ./bin/qserver

## Implicit Section: change the following only when necessary.
##==============================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .cc

# The header file types.
HDREXTS = .h 

# The pre-processor and compiler options.
# Users can override those variables from the command line.
CFLAGS  =
# CXXFLAGS= -std=c++0x
CXXFLAGS=
# The C program compiler.
CC     = gcc

# The C++ program compiler.
CXX    = g++

# Un-comment the following line to compile C programs as C++ ones.
#CC     = $(CXX)

# The command used to delete file.
RM     = rm -f

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==============================================================================
SHELL   = /bin/sh
EMPTY   = 
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
  ifeq ($(PROGRAM),)
    PROGRAM = a.out
  endif
endif
ifeq ($(SRCDIRS),)
  SRCDIRS = .
endif

## display INCLUDEDIRS --ken
ifeq ($(INCDIRS),)
  INCDIRS = .
endif

SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(INCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS    = $(addsuffix .o, $(basename $(SOURCES)))
DEPS    = $(OBJS:.o=.d)

####++++++++++++++++++++++++++++++++++
## Customizable May Change Section: 
# Two step to define the suffixs of the temporary Files which you want to delete.
# This is First Step: deinfe it. (The second step see below.) --ken
TMPFILES    = $(addsuffix .xxxx, $(basename $(SOURCES)))
####++++++++++++++++++++++++++++++++++

## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )
DEPEND      = $(CC)  $(DEP_OPT)  $(MY_CFLAGS) $(addprefix -I,$(INCDIRS)) $(CFLAGS) $(CPPFLAGS)
DEPEND.d    = $(subst -g ,,$(DEPEND))
COMPILE.c   = $(CC)  $(MY_CFLAGS) $(addprefix -I,$(INCDIRS)) $(CFLAGS)   $(CPPFLAGS) -c
COMPILE.cxx = $(CXX) $(MY_CFLAGS) $(addprefix -I,$(INCDIRS)) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.c      = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) $(LDFLAGS)
LINK.cxx    = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

.PHONY: all objs tags ctags clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: $(PROGRAM)

####++++++++++++++++++++++++++++++++++
## Customizable may change Section: 
# add "./$(PROGRAM)" that can run the executable file after make.
# you can enble/disable it. --ken
	./$(PROGRAM)
####++++++++++++++++++++++++++++++++++

# Rules for creating dependency files (.d).
#------------------------------------------

%.d:%.c
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.C
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cc
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cpp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.CPP
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.c++
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cxx
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

%.o:%.c
	$(COMPILE.c) $< -o $@

%.o:%.C
	$(COMPILE.cxx) $< -o $@

%.o:%.cc
	$(COMPILE.cxx) $< -o $@

%.o:%.cpp
	$(COMPILE.cxx) $< -o $@

%.o:%.CPP
	$(COMPILE.cxx) $< -o $@

%.o:%.c++
	$(COMPILE.cxx) $< -o $@

%.o:%.cp
	$(COMPILE.cxx) $< -o $@

%.o:%.cxx
	$(COMPILE.cxx) $< -o $@

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

# Rules for generating the executable.
#-------------------------------------
$(PROGRAM):$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(LINK.c)   $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
else                            # C++ program
	$(LINK.cxx) $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
endif

ifndef NODEP
ifneq ($(DEPS),)
  sinclude $(DEPS)
endif
endif

clean:
	$(RM) $(OBJS) $(PROGRAM) $(PROGRAM).exe

distclean: clean
	$(RM) $(DEPS) TAGS

####++++++++++++++++++++++++++++++++++
## Customizable May Change Section: 
# Two step to define the suffixs of the temporary Files which you want to delete.
# This is Second Step: delete the suffixes of the temporary file which define in 
# the first step. or any other file you that want to delete in here.
# (The first step is in above.) --ken
cleanall: clean
	$(RM) $(DEPS) $(TMPFILES)
####++++++++++++++++++++++++++++++++++

# Show help.
help:
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 0.5'
	@echo 'Copyright (C) 2007, 2008 whyglinux <whyglinux@hotmail.com>'
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  NODEP=yes make without generating dependencies.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  ctags     create ctags for VI editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  cleanall  clean objects, executable, dependencies and any customize define.'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'
	@echo
	@echo 'Report bugs to <whyglinux AT gmail DOT com>.'
	@echo 'This [Makefile_For_GNU_C_CXX_depstar_release] is renew by ken, from china!'
	@echo
	
# Show variables (for debug use only.)
show:
	@echo '++ PROGRAM     :' $(PROGRAM)
	@echo '++ SRCDIRS     :' $(SRCDIRS)
	@echo '++ INCDIRS     :' $(INCDIRS)
	@echo '++ HEADERS     :' $(HEADERS)
	@echo '++ SOURCES     :' $(SOURCES)
	@echo '++ SRC_CXX     :' $(SRC_CXX)
	@echo '++ OBJS        :' $(OBJS)
	@echo '++ DEPS        :' $(DEPS)
	@echo '++ DEPEND      :' $(DEPEND)
	@echo '++ COMPILE.c   :' $(COMPILE.c)
	@echo '++ COMPILE.cxx :' $(COMPILE.cxx)
	@echo '++ link.c      :' $(LINK.c)
	@echo '++ link.cxx    :' $(LINK.cxx)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
################################################################################
