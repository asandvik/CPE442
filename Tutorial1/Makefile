
CC = gcc
LDFLAGS = 
CFLAGS = -g -Wall
SRC = helloworld.c helloworld.h
OUT = helloworld

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	-rm $(OUT) *.o *~