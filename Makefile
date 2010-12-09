build/FileRequestor.o: build-dir src/FileRequestor.c src/FileRequestor.h
	gcc -c -o build/FileRequestor.o src/FileRequestor.c

build/Tokenizer.o: build-dir src/Tokenizer.c src/Tokenizer.h
	gcc -c -o build/Tokenizer.o src/Tokenizer.c

build/FileRequestorTest: build-dir build/FileRequestor.o
	gcc -o build/FileRequestorTest src/FileRequestorTest.c build/FileRequestor.o

build/TokenizerTest: build-dir build/Tokenizer.o
	gcc -o build/TokenizerTest src/TokenizerTest.c build/Tokenizer.o

build/stats: build-dir
	gcc -o build/stats src/stats.c

build/fusetest: build-dir
	gcc -Wall `pkg-config fuse --cflags --libs` src/fusetest.c -o build/fusetest

build-dir:
	mkdir -p build

clean:
	rm build/*

test: build/FileRequestorTest build/TokenizerTest
	build/FileRequestorTest
	build/TokenizerTest

stats: build/stats
	build/stats

all: build/fusetest build/FileRequestorTest
