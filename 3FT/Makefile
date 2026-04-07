#--------------------------------------------------------------------
# Makefile for the File Tree (FT) Node Module
# Author: Ty Lipscomb and Tom Wang
#--------------------------------------------------------------------

# Macros
CC = gcc217
# Use -g for debugging with gdb or valgrind
CFLAGS = -g

# The default target
all: testNode

# Link the executable
# Note: path.o and dynarray.o must be present in your directory
testNode: testNode.o nodeFT.o path.o dynarray.o
	$(CC) $(CFLAGS) testNode.o nodeFT.o path.o dynarray.o -o testNode

# Compile testNode.o
testNode.o: testNode.c nodeFT.h path.h a4def.h
	$(CC) $(CFLAGS) -c testNode.c

# Compile nodeFT.o
# This depends on all headers it includes
nodeFT.o: nodeFT.c nodeFT.h path.h dynarray.h a4def.h
	$(CC) $(CFLAGS) -c nodeFT.c

# Clean up the directory
clean:
	rm -f testNode *.o