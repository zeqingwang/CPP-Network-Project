
CC = g++
CFLAGS = -Wall -g


all: serverM serverS serverD serverU client


extra: CFLAGS += -DEXTRA_ENCRYPT
extra: serverM serverS serverD serverU client


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


clean:
	rm -f serverM serverS serverD serverU client
