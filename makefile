################################
# makeFile Projet
#   Serie 1
#
#				   Haba, Gabriel
#			 Nuez Soriano, Diego
################################

CFLAGS=-g
CC=/usr/bin/cc

all: serveur joueur

joueur: joueur.o socket.o shared_memory.o
	$(CC) $(CFLAGS) -o joueur joueur.o socket.o shared_memory.o

serveur: serveur.o socket.o shared_memory.o
	$(CC) $(CFLAGS) -o serveur serveur.o socket.o shared_memory.o

joueur.o: joueur.c joueur.h common.h
		$(CC) $(CFLAGS) -c joueur.c

serveur.o: serveur.c serveur.h common.h
		$(CC) $(CFLAGS) -c serveur.c

shared_memory.o: shared_memory.c shared_memory.h common.h
		$(CC) $(CFLAGS) -c shared_memory.c

socket.o: socket.c socket.h common.h
		$(CC) $(CFLAGS) -c socket.c

clean :
	rm serveur
	rm joueur
	rm *.o
	./remove_ipc.sh
