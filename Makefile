TARGET = prog
CC = cc
CFLAGS = -g -Wall
LIBS = -L/usr/local/lib				#replace with output of pg_config --libdir
INCLUDE = -I/usr/local/bin/pgsql/include 	#replace with output of pg_config --includedir
#If you are on Ubuntu (and potentially other distros that come with postgresql by default)
#Then go into bank.h and uncomment the two include statements that are needed for Ubuntu
#to work. Follow the instructions from there

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
	$(CC) $(CFLAGS) -c $(INCLUDE) $< -o $@

# PRECIOUS files are not deleted like usual intermediary files
.PRECIOUS: $(TARGET) $(OBJECTS)

# Generates output file
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ -lpq -lcrypto

clean:
	-rm -f *.o
	-rm -f $(TARGET)
