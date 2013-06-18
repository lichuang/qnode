#!/bin/sh

#HEAPCHECK=normal ./bin/qserver
valgrind --tool=memcheck --leak-check=yes ./bin/qserver
