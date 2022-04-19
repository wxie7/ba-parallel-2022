CC	 = gcc
CFLAGS	 = 
CSTD	 = gnu90
LIBS	 = pthread
BUILDDIR = build
TARGET	 = cyk
SOURCES	 = cyk.c threadpool.c
OBJECTS	 = $(patsubst %.c,$(BUILDDIR)/%.o,$(SOURCES))

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $^ -l$(LIBS) -o $@

$(OBJECTS): $(BUILDDIR)/%.o : %.c
	$(CC) -std=$(CSTD) -c $(CFLAGS) $< -o $@
