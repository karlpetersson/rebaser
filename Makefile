OUT=build/rex
INC=src
CFLAGS=-I$(INC)
ODIR=src
TDIR=tests

_DEPS = parser.h sds.h arrlist.h commit.h term.h colors.h gui.h
DEPS = $(patsubst %,$(INC)/%,$(_DEPS))

_OBJ = main.o parser.o arrlist.o sds.o commit.o term.o gui.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_TESTS = $(wildcard $(TDIR)/*.c)
TESTS = $(patsubst %.c,%,$(_TESTS))

$(ODIR)/%.o: %.c $(DEPS)
	gcc -c -o $@ $< $(CFLAGS)

default: $(OBJ)
	gcc -o $(OUT) $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INC)/*~
