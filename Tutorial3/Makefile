CC = g++
CFLAGS = -O0 
LDFLAGS = `pkg-config --cflags --libs opencv4` 
SRC = imgp.cpp imgp.hpp
OUT = imgp


all: imgp tut2

imgp: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

tut2: tut2.cpp
	$(CC) $(CFLAGS) tut2.cpp -o $@ $(LDFLAGS)

clean:
	rm $(OUT) 
