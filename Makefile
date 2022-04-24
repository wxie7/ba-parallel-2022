CC       = gcc
OPTLEVEL = 
CFLAGS   = -fopenmp $(OPTLEVEL)
CSTD     = gnu90
LLIBS    = 
BUILDDIR = build
TARGET   = cyk
SOURCES  = cyk.c
OBJECTS  = $(patsubst %.c,$(BUILDDIR)/%.o,$(SOURCES))

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $^ $(LIBS) $(CFLAGS) -o $@

$(OBJECTS): $(BUILDDIR)/%.o : %.c
	$(CC) -std=$(CSTD) -c $(CFLAGS) $< -o $@ $(OPTLEVEL)
