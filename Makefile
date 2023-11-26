CC       = gcc
CFLAGS   = -Wall -I$(INCLUDEDIR)

INCLUDEDIR = include
SOURCEDIR  = src
BUILDDIR   = build

EXE = LoxMin
SRC = $(wildcard $(SOURCEDIR)/*.c)
HDR = $(wildcard $(INCLUDEDIR)/*.h)

all: dir $(BUILDDIR)/$(EXE)

dir:
	if not exist $(BUILDDIR) mkdir $(BUILDDIR)

$(BUILDDIR)/$(EXE): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $^ -o $@
