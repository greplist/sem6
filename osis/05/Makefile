CC = g++
CFLAGS = -c -std=gnu++11 -Wall -Wextra
LDFLAGS = -lc

LABS = $(wildcard *.c)
OBJECTS = $(LABS:.c=.o)
BINARIES = $(OBJECTS:.o=.bin)

all: clean $(OBJECTS) $(BINARIES)
	echo "Compiled!"

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.bin: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	-rm -r *.o *.bin *~ *.new
