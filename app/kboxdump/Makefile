

EXE = kboxdump
CROSS =

CC = aarch64-openwrt-linux-gcc

LIBS := -lpthread

PWD       = $(shell pwd)
LDDINC = $(PWD)/../include
INCLUDE := -I$(LDDINC)

#CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes
CFLAGS =	-O2 -fomit-frame-pointer -Wall   -pipe #-pedantic

$(EXE):kboxdump.o
	$(CC) -o $(EXE) kboxdump.o $(LIBS)

kboxdump.o:kboxdump.c
	$(CC) -c $(CFLAGS) $(INCLUDE) kboxdump.c

clean:
	rm $(EXE) *.o

