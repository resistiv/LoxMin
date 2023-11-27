CC       = gcc
CFLAGS   = -Wall -I$(INCLUDEDIR)

INCLUDEDIR = include
SOURCEDIR  = src
BUILDDIR   = build

EXE = LoxMin
SRC = $(wildcard $(SOURCEDIR)/*.c)
# HDR = $(wildcard $(INCLUDEDIR)/*.h)

all: $(BUILDDIR)/$(EXE)

$(BUILDDIR)/$(EXE): $(SRC) # $(HDR)
	$(CC) $(CFLAGS) $^ -o $@
