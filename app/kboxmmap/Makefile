

EXE = kboxmmap
CROSS =

CC = aarch64-openwrt-linux-gcc

LIBS := -lpthread

PWD       = $(shell pwd)
LDDINC = $(PWD)/../include
INCLUDE := -I$(LDDINC)

#CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes
CFLAGS =	-O2 -fomit-frame-pointer -Wall   -pipe #-pedantic

$(EXE):kboxmmap.o
	$(CC) -o $(EXE) kboxmmap.o $(LIBS)

kboxmmap.o:kboxmmap.c
	$(CC) -c $(CFLAGS) $(INCLUDE) kboxmmap.c

clean:
	rm $(EXE) *.o

