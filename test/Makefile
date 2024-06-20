CPPFLAGS = -std=c++11 -I..

.PHONY: all
all: test_paiv_json.cpp
	mkdir -p bin
	$(CC) $(CPPFLAGS) -O0 -g -o bin/test $+

.PHONY: test
test: all
	bin/test

.PHONY: clean
clean:
	rm -r bin
