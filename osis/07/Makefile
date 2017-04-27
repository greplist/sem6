CC = g++
CFLAGS = -c -std=gnu++11 -Wall -Wextra
LDFLAGS = -L./../suplib -lsuplib -lc -Wl,-rpath ./../suplib

LABS = $(wildcard *.c)
OBJECTS = $(LABS:.c=.o)
BINARIES = $(OBJECTS:.o=.bin)

all: clean build_suplib $(OBJECTS) $(BINARIES)
	echo -e "Compiled!\n"

build_suplib:
	make -C ./../suplib

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.bin: %.o
	$(CC) $< $(LDFLAGS) -o $@

clean:
	rm -rf *.o *.bin *~ *.new
