IDIR=include
SDIR=src
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=$(SDIR)/obj
LDIR =lib

LIBS=-lm -lncurses

# Add .h include files here
_DEPS = 

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# Add .o include file deps here
_OBJ = 

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