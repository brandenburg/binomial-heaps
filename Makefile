
CFLAGS = -Wall -Wextra -Werror -ansi -std=c99 -pedantic

ALL = htest ihtest

.PHONY: clean all

all: ${ALL}

clean:
	rm -f ${ALL} *.pyc

htest: htest.c

ihtest: ihtest.c
