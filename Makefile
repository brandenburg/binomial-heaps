
CFLAGS = -Wall -Wextra -Werror -ansi -std=c99 -pedantic

ALL = htest htest-next ihtest

.PHONY: clean all

all: ${ALL}

clean:
	rm -f ${ALL} *.pyc

htest: htest.c
htest-next: htest-next.c

ihtest: ihtest.c
