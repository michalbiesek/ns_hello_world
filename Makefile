CC      = gcc
CFLAGS  = -g
RM      = rm


default: all

all: ns_example

ns_example: ns_example.c
	$(CC) $(CFLAGS) -o ns_example ns_example.c -lrt

clean:
	$(RM) ns_example
