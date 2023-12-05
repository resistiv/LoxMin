CC       = gcc
CFLAGS   = -Wall -I$(INCLUDEDIR) -O3

INCLUDEDIR = include
SOURCEDIR  = src

EXE = LoxMin
SRC = $(wildcard $(SOURCEDIR)/*.c)

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
