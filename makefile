################################
# makeFile Projet
#   Serie 1
#
#				   Haba, Gabriel
#			 Nuez Soriano, Diego
################################

CFLAGS=-g

all: serveur joueur

joueur: joueur.o socket.o
	cc $(CFLAGS) -o joueur joueur.o socket.o

serveur: serveur.o socket.o
	cc $(CFLAGS) -o serveur serveur.o socket.o

joueur.o: joueur.c joueur.h common.h
		cc $(CFLAGS) -c joueur.c

serveur.o: serveur.c serveur.h common.h
		cc $(CFLAGS) -c serveur.c

socket.o: socket.c socket.h common.h
		cc $(CFLAGS) -c socket.c

clean :
	rm serveur
	rm joueur
	rm *.o
