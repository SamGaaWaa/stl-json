all: test.out

test.out: test/test.cpp json.hpp
	g++  -O0 -g  -std=c++11 -I . $^ -o $@

clean:
	rm test.out
