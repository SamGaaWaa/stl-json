all: build/test.out

build/test.out: test.cpp json.hpp
	g++ -std=c++23 -O0 -g $^ -o $@

clean:
	rm build/* -rf