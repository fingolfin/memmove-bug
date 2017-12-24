all: memmove report

memmove: memove-bug
	./memove-bug || echo "memmove is BUGGY"

nomemmove: memove-bug
	./memove-bug -m

memove-bug: memmove-bug.c
	cc -m32 -O2 -g -Wall -o $@ $<

report: memove-bug
	@echo "Details for your system:"
	@`ldd ./memove-bug | grep libc | sed 's;[^/]*\(/[^ ]*\).*;\1;'` | head -n 1
	@$(CC) --version | head -n 1
	@uname -rv

.PHONY: all memmove nomemmove report
