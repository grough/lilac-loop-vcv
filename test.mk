.PHONY: test

test.out: test/test.cpp
	$(CXX) -std=c++11 -Isrc/ test/test.cpp -o test/test.out

test: test.out
	./test/test.out
