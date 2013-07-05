#!/bin/sh

cat ./qserver.pid | xargs kill -USR1
