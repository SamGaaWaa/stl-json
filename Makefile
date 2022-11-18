all: test.out

test.out: test.cpp json.hpp
	g++  -O3 $^ -o $@

clean:
	rm test.out
