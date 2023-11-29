CC       = gcc
CFLAGS   = -Wall -Wextra -I$(INCLUDEDIR)

INCLUDEDIR = include
SOURCEDIR  = src

EXE = LoxMin
SRC = $(wildcard $(SOURCEDIR)/*.c)

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
