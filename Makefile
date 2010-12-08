fusetest:
	gcc -Wall `pkg-config fuse --cflags --libs` src/fusetest.c -o build/fusetest

clean:
	rm build/*

all: fusetest