CC       = gcc
CFLAGS   = -fopenmp
CSTD     = gnu90
LLIBS    = -lpthread
BUILDDIR = build
TARGET   = cyk
SOURCES  = cyk.c
OBJECTS  = $(patsubst %.c,$(BUILDDIR)/%.o,$(SOURCES))

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $^ $(LLIBS) $(CFLAGS) -o $@

$(OBJECTS): $(BUILDDIR)/%.o : %.c
	$(CC) -std=$(CSTD) -c $(CFLAGS) $< -o $@ $(LLIBS)
