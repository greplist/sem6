CC = g++
CFLAGS = -c -O2 -std=c++14 -g #-Wall -Wextra

1:
	lex 1.lex
	$(CC) $(CFLAGS) lex.yy.c -o lex.yy.c.o
	$(CC) lex.yy.c.o -o 1.bin
	./1.bin test.py

clean:
	rm -rf *.o *.yy.c *.tab.c *.bin
