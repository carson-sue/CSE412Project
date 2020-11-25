TARGET = prog
CC = cc
CFLAGS = -g -Wall
LIBS = -L/usr/local/lib

# PHONY names are not associated with files
.PHONY: default all clean

# Possible to use "make" "make default" and "make all"
default: $(TARGET)
all: default

# Specifies files
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

# $@ is the file being generated, $< is the first prerequisite
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -I/usr/local/bin/pgsql/include $< -o $@

# PRECIOUS files are not deleted like usual intermediary files
.PRECIOUS: $(TARGET) $(OBJECTS)

# Generates output file
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ -lpq

clean:
	-rm -f *.o
	-rm -f $(TARGET)
