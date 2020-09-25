

EXE = mmapkbox
CROSS =

CC = aarch64-openwrt-linux-gcc

LIBS := -lpthread

PWD       = $(shell pwd)
LDDINC = $(PWD)/../include
INCLUDE := -I$(LDDINC)

#CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes
CFLAGS =	-O2 -fomit-frame-pointer -Wall   -pipe #-pedantic

$(EXE):main.o
	$(CC) -o $(EXE) main.o $(LIBS)

main.o:main.c
	$(CC) -c $(CFLAGS) $(INCLUDE) main.c

clean:
	rm $(EXE) *.o

