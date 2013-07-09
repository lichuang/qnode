#!/bin/sh

valgrind --tool=memcheck --leak-check=full --suppressions=./valgrind/qnode.supp ./bin/qserver
