./test/test.out: ./test/main.cc ./test/cJSON.c
	g++ -std=c++23 -DNDEBUG -O3 $^ -o $@

test: ./test/test.out
	@./test/test.out
	@rm ./test/json/big.json

clean:
	rm ./test/test.out