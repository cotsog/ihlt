all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

test: clean
	CFLAGS=-g LDFLAGS=-g $(MAKE)
	set -x; { /usr/bin/timeout 3m strace -o o.log -f -s 999 prove -f && true; } || { cat o.log; false; }
	echo "Test passed."

.PHONY: all clean test
