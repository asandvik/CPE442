CC = g++
CFLAGS = -g -Werror -std=c++11 -O2 -lpthread
LDFLAGS = -pthread `pkg-config --cflags --libs opencv4 papi` 
SRC = imgp.cpp imgp.hpp
OUT = imgp

all: imgp

imgp: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

clean:
	rm $(OUT) 
