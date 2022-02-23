CC = gcc
SRC = src/main.c
OBJ = ${SRC:.c=.o}
LDFLAGS = -lpng -lm

smilegen: ${SRC}
	${CC} ${SRC} -o $@ ${LDFLAGS}

clean:
	rm -f ${OBJ} smilegen
	rm -f output/*.png

.PHONY: clean