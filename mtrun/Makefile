CC = gcc
CFLAGS = -c -O2 -std=c99 -Wall -Wextra -g

SOURCES = lex.yy.c
OBJECTS = $(SOURCES:.c=.o)

INPUT = test.py
EXECUTABLE = analyzer.bin

all: $(EXECUTABLE)
	./$(EXECUTABLE) $(INPUT)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE)

.cpp.o: .cpp
	$(CC) $(CFLAGS) $< -o $@

lex.yy.c: syntax.lex
	lex $<

clean:
	rm -rf *.o *.yy.c *.bin
