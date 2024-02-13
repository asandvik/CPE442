CC = g++
CFLAGS = -g -Werror -std=c++11 -O0 -lpthread
LDFLAGS = -pthread `pkg-config --cflags --libs opencv4` 
SRC = imgp.cpp imgp.hpp
OUT = imgp

all: imgp tut4

imgp: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

tut4: tut4.cpp
	$(CC) $(CFLAGS) tut4.cpp -o $@ $(LDFLAGS)

clean:
	rm $(OUT) 
