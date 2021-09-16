IDIR=include
SDIR=src
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Werror -D DEBUG

ODIR=$(SDIR)/obj
LDIR =lib

LIBS=-lm -lncurses

# Add .h include files here
_DEPS = libs.h error_handlers.h utils.h vector.h prompt.h parsing.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# Add .o include file deps here
_OBJ = error_handlers.o utils.o shell.o vector.o prompt.o parsing.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -g -o $@ $< $(CFLAGS)

# make shell
shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# make clean
.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 