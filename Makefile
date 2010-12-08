build/FileRequestor.o: build-dir
	gcc -c -o build/FileRequestor.o src/FileRequestor.c

build/FileRequestorTest: build-dir build/FileRequestor.o
	gcc -o build/FileRequestorTest src/FileRequestorTest.c build/FileRequestor.o

build/fusetest: build-dir
	gcc -Wall `pkg-config fuse --cflags --libs` src/fusetest.c -o build/fusetest

build-dir:
	mkdir -p build

clean:
	rm build/*

test: build/FileRequestorTest
	build/FileRequestorTest

all: build/fusetest build/FileRequestorTest
