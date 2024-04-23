# Compiler and flags
CC = g++
CFLAGS = -Wall -g

# Target executables
all: serverM serverS serverD client

# Compilation recipes
serverM: serverM.cpp
	$(CC) $(CFLAGS) -o serverM serverM.cpp

serverS: serverS.cpp
	$(CC) $(CFLAGS) -o serverS serverS.cpp
serverD: serverD.cpp
	$(CC) $(CFLAGS) -o serverD serverD.cpp	


client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

# Clean up
clean:
	rm -f serverM serverS serverD client
