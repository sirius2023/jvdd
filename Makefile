## Toplevel Makefile

CC := gcc
JUDASINC := ./judas
JUDASOBJ := $(JUDASINC)/timer.o $(JUDASINC)/timerasm.o $(JUDASINC)/judaslib.a
#CFLAGS := -W -Wall -O3 -I.
#CFLAGS := -DALLEGRO_GUESS_INTTYPES_OK -Wl,--allow-multiple-definition -Wfatal-errors -Wall -O3 -I.
CFLAGS := -DALLEGRO_GUESS_INTTYPES_OK -Wl,--allow-multiple-definition -Wfatal-errors -Wall -g -I. -I$(JUDASINC)

VERSION := 1.5
# (note to self: remember to edit loadpng.h as well)

LIB := libldpng.a

all: game


ifdef DJDIR
include Makefile.dj
else
ifdef MINGDIR
include Makefile.mgw
else
include Makefile.unx
endif
endif

#--------------------------------------------------
.PHONY: lib

LDPNGLIB := LIBLDPNG.A

lib: $(LDPNGLIB)
	cd loadpng
	make
	make install
	cd ..



#--------------------------------------------------
.PHONY: game

game: allejv$(EXE)

allejv$(EXE): allejv.cpp $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS) $(JUDASOBJ)


#--------------------------------------------------
# Note: this requires rm
.PHONY: clean

clean:
	rm -f allejv.exe
