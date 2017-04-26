#!/bin/bash

set -e

CC=g++
CFLAGS="-c -O2 -std=c++14 -g" #-Wall -Wextra

flex 1.lex
${CC} ${CFLAGS} lex.yy.c -o lex.yy.c.o
${CC} lex.yy.c.o -o 2.bin
./2.bin test.py
