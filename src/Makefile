

CFLAGS= -O
LDFLAGS= -s -lpthread


all: skl


skl.o: skl.h

config.o: config.h skl.h

proceso.o: proceso.h config.h

log.o: log.h


skl: skl.o config.o proceso.o log.o
	cc ${LDFLAGS} -o skl skl.o config.o proceso.o log.o

