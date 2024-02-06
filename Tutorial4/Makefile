CC = g++
CFLAGS = -O0 -g
LDFLAGS = -pthread `pkg-config --cflags --libs opencv4` 
SRC = imgp.cpp imgp.hpp
OUT = imgp

all: imgp thread

imgp: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

thread: thread.cpp
	$(CC) $(CFLAGS) thread.cpp -o $@ $(LDFLAGS)

clean:
	rm $(OUT) 
