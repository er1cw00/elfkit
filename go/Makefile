.PHONY: all test clean

all:
	cd elfkit && go build

clean:


test: all
	cd elfkit && LD_LIBRARY_PATH=../../ DYLD_LIBRARY_PATH=../../ go test
