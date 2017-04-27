CC = g++
CFLAGS = -c -std=gnu++11 -Wall -Wextra
LDFLAGS = -lc

OBJECTS = astring.o aio.o sighand.o
BINARY = sighand.bin

all: clean $(BINARY)
	echo "Compiled!"

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

sighand.bin: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


clean:
	rm -rf *.o *.bin *~ *.new
