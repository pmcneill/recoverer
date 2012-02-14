PRINT_SIZE=32
CFLAGS=-g -Wall -DPRINT_SIZE=$(PRINT_SIZE)

all: fingerprint recoverer

fingerprint: fingerprint.o match.o
	$(CC) $(CFLAGS) -o fingerprint fingerprint.o match.o

recoverer: recoverer.o match.o
	$(CC) $(CFLAGS) -o recoverer recoverer.o match.o
