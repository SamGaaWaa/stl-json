all: test.out

test.out: test/test.cpp json.hpp
	g++  -O3   -std=c++11 -I . $^ -o $@

clean:
	rm test.out
