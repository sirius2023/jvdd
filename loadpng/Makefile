## Toplevel Makefile

CC := gcc
JUDASINC := ./judas
#CFLAGS := -W -Wall -O3 -I.
#CFLAGS := -DALLEGRO_GUESS_INTTYPES_OK -Wl,--allow-multiple-definition -Wfatal-errors -Wall -O3 -I.
CFLAGS := -DALLEGRO_GUESS_INTTYPES_OK -Wl,--allow-multiple-definition -Wfatal-errors -Wall -g -I. -I$(JUDASINC)

VERSION := 1.5
# (note to self: remember to edit loadpng.h as well)

LIB := libldpng.a

all: lib 

libdir := ..
includedir := ..


#--------------------------------------------------
.PHONY: lib

lib: $(LIB)

$(LIB): loadpng.o savepng.o regpng.o
	ar rs $@ $^

#--------------------------------------------------
.PHONY: pinkpng

pinkpng$(EXE): pinkpng.c $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS)

#--------------------------------------------------
# Note: this requires rm
.PHONY: clean

clean:
	rm -f $(EXAMPLES) examples/saved.png
	rm -f loadpng.o savepng.o regpng.o $(LIB) $(SHAREDLIB)

#--------------------------------------------------
.PHONY: install

install: $(LIB)
	copy loadpng.h $(subst /,\,$(includedir))
	copy $(LIB) $(subst /,\,$(libdir))
