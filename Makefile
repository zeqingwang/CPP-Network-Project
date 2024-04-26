# Compiler and flags
CC = g++
CFLAGS = -Wall -g

# Target executables
all: serverM serverS serverD serverU client

# Extra compilation with encryption
extra: CFLAGS += -DEXTRA_ENCRYPTION
extra: serverM serverS serverD serverU client

# Compilation recipes
serverM: serverM.cpp
	$(CC) $(CFLAGS) -o serverM serverM.cpp

serverS: serverS.cpp
	$(CC) $(CFLAGS) -o serverS serverS.cpp
serverD: serverD.cpp
	$(CC) $(CFLAGS) -o serverD serverD.cpp	
serverU: serverU.cpp
	$(CC) $(CFLAGS) -o serverU serverU.cpp	


client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

# Clean up
clean:
	rm -f serverM serverS serverD serverU client
