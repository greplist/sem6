#!/bin/bash

set -e

CC=g++
CFLAGS="-c -O2 -g" #-Wall -Wextra

flex --header-file=lex.yy.h 2.lex
yacc -d 2.yacc
${CC} ${CFLAGS} tree.cpp -o tree.cpp.o
${CC} ${CFLAGS} lex.yy.c -o lex.yy.c.o
${CC} ${CFLAGS} y.tab.c -o y.tab.c.o
${CC} lex.yy.c.o y.tab.c.o tree.cpp.o -o 2.bin
./2.bin test.py
