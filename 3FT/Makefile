#--------------------------------------------------------------------
# Makefile for the File Tree (FT) Node Module
# Author: Ty Lipscomb and Tom Wang
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# Makefile for the File Tree (FT) Module
# Author: Ty Lipscomb and Tom Wang
#--------------------------------------------------------------------

# Macros
CC = gcc217
CFLAGS = -g

# Default target
all: myft

# Link the final executable
# Note: path.o and dynarray.o must be present in your directory
myft: ft_client.o ft.o nodeFT.o path.o dynarray.o
	$(CC) $(CFLAGS) ft_client.o ft.o nodeFT.o path.o dynarray.o -o myft

# Compile the client
ft_client.o: ft_client.c ft.h a4def.h
	$(CC) $(CFLAGS) -c ft_client.c

# Compile the FT manager
ft.o: ft.c ft.h nodeFT.h path.h a4def.h
	$(CC) $(CFLAGS) -c ft.c

# Compile the Node module
nodeFT.o: nodeFT.c nodeFT.h path.h dynarray.h a4def.h
	$(CC) $(CFLAGS) -c nodeFT.c

# Clean target to remove generated files
clean:
	rm -f myft *.o