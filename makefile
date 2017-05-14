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

joueur: joueur.o socket.o ipc.o
	$(CC) $(CFLAGS) -o joueur joueur.o socket.o ipc.o

serveur: serveur.o socket.o ipc.o
	$(CC) $(CFLAGS) -o serveur serveur.o socket.o ipc.o

joueur.o: joueur.c joueur.h common.h
		$(CC) $(CFLAGS) -c joueur.c

serveur.o: serveur.c serveur.h common.h
		$(CC) $(CFLAGS) -c serveur.c

ipc.o: ipc.c ipc.h common.h
		$(CC) $(CFLAGS) -c ipc.c

socket.o: socket.c socket.h common.h
		$(CC) $(CFLAGS) -c socket.c

clean :
	rm serveur
	rm joueur
	rm *.o
	./remove_ipc.sh
