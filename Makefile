# These flags are to make fuse happy:
GCC_FLAGS = -Wall `pkg-config fuse --cflags --libs`
CC = gcc ${GCC_FLAGS}

GCC_OBJ_FLAGS = -Wall `pkg-config fuse --cflags`
CC_OBJ = gcc ${GCC_OBJ_FLAGS}

build/FileRequestor.o: src/FileRequestor.c src/FileRequestor.h
	mkdir -p build
	${CC_OBJ} -c -o build/FileRequestor.o src/FileRequestor.c

build/Tokenizer.o: src/Tokenizer.c src/Tokenizer.h
	mkdir -p build
	${CC_OBJ} -c -o build/Tokenizer.o src/Tokenizer.c

build/FileRequestorTest: build/FileRequestor.o build/Tokenizer.o src/FileRequestorTest.c
	mkdir -p build
	${CC} -o build/FileRequestorTest src/FileRequestorTest.c build/FileRequestor.o build/Tokenizer.o

build/FileRequestorTest2: build/FileRequestor.o build/Tokenizer.o src/FileRequestorTest2.c
	mkdir -p build
	${CC} -o build/FileRequestorTest2 src/FileRequestorTest2.c build/FileRequestor.o build/Tokenizer.o

build/TokenizerTest: build/Tokenizer.o src/TokenizerTest.c
	mkdir -p build
	${CC} -o build/TokenizerTest src/TokenizerTest.c build/Tokenizer.o

build/stats:
	mkdir -p build
	${CC} -o build/stats src/stats.c

build/fusetest:
	mkdir -p build
	${CC} src/fusetest.c -o build/fusetest

clean:
	rm -rf build

test: build/TokenizerTest build/FileRequestorTest build/FileRequestorTest2
	build/TokenizerTest
	build/FileRequestorTest
	build/FileRequestorTest2

stats: build/stats
	build/stats

all: build/fusetest build/FileRequestorTest
