build/FileRequestor.o: src/FileRequestor.c src/FileRequestor.h
	mkdir -p build
	gcc -c -o build/FileRequestor.o src/FileRequestor.c

build/Tokenizer.o: src/Tokenizer.c src/Tokenizer.h
	mkdir -p build
	gcc -c -o build/Tokenizer.o src/Tokenizer.c

build/FileRequestorTest: build/FileRequestor.o build/Tokenizer.o src/FileRequestorTest.c
	mkdir -p build
	gcc -o build/FileRequestorTest src/FileRequestorTest.c build/FileRequestor.o build/Tokenizer.o

build/TokenizerTest: build/Tokenizer.o src/TokenizerTest.c
	mkdir -p build
	gcc -o build/TokenizerTest src/TokenizerTest.c build/Tokenizer.o

build/stats:
	mkdir -p build
	gcc -o build/stats src/stats.c

build/fusetest:
	mkdir -p build
	gcc -Wall `pkg-config fuse --cflags --libs` src/fusetest.c -o build/fusetest

clean:
	rm -rf build

test: build/FileRequestorTest build/TokenizerTest
	build/FileRequestorTest
	build/TokenizerTest

stats: build/stats
	build/stats

all: build/fusetest build/FileRequestorTest
