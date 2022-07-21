CC      = gcc
CFLAGS  = -g
RM      = rm


default: all

all: ns_example

ns_example: ns_example.c
	$(CC) $(CFLAGS) -o ns_example ns_example.c

clean:
	$(RM) ns_example
