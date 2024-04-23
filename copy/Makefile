# Compiler and flags
CC = g++
CFLAGS = -Wall -g

# Target executables
all: serverM serverS client

# Compilation recipes
serverM: serverM.cpp
	$(CC) $(CFLAGS) -o serverM serverM.cpp

serverS: serverS.cpp
	$(CC) $(CFLAGS) -o serverS serverS.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

# Clean up
clean:
	rm -f serverM serverS client
