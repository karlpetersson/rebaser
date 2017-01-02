OUT=build/ribs
INC=src
CFLAGS=-I$(INC)
ODIR=src
TDIR=tests

_DEPS = parser.h sds.h arraylist.h commit.h term.h colors.h gui.h
DEPS = $(patsubst %,$(INC)/%,$(_DEPS))

_OBJ = main.o parser.o arraylist.o sds.o commit.o term.o gui.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_TESTS = $(wildcard $(TDIR)/*.c)
TESTS = $(patsubst %.c,%,$(_TESTS))

$(ODIR)/%.o: %.c $(DEPS)
	gcc -c -o $@ $< $(CFLAGS)

default: $(OBJ)
	gcc -o $(OUT) $^ $(CFLAGS)

.PHONY: clean
.PHONY: install

install:
	cp $(OUT) /usr/local/bin
	
clean:
	rm -f $(ODIR)/*.o *~ core $(INC)/*~
