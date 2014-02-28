TARGETS=client server 

CC=gcc
CCOPTS=-g -Wall

.PHONY: all clean

all: $(TARGETS)

clean: 
	rm -f $(TARGETS)
%: %.c
	$(CC) $(CCOPTS) -o $@ $<
