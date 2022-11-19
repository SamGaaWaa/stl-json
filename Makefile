all: test.out

test.out: test.cpp json.hpp
	g++  -O0 -g -fpermissive $^ -o $@

clean:
	rm test.out
