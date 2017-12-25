all: memmove report

memmove: memmove-bug
	./memmove-bug || echo "memmove is BUGGY"

nomemmove: memmove-bug
	./memmove-bug -m

memmove-bug: memmove-bug.c
	cc -m32 -O2 -g -Wall -o $@ $<

report: memmove-bug
	@echo "Details for your system:"
	@`ldd ./memmove-bug | grep libc | sed 's;[^/]*\(/[^ ]*\).*;\1;'` | head -n 1
	@$(CC) --version | head -n 1
	@uname -rv

clean:
	rm -f memmove-bug memove-bug

.PHONY: all memmove nomemmove report clean
