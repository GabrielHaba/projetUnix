/****************************************
 *	Fichier d'entete commun au server
 *  et au client
 *	Projet UNIX
 *  Serie 1
 *							Haba, Gabriel
 *					  Nuez Soriano, Diego
 ****************************************/

#include	<netdb.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#include	<sys/socket.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<unistd.h>
#include 	<signal.h>
#include	<netinet/in.h>
#include	<time.h>
#include	<errno.h>
#include	<sys/select.h>

#ifndef _COMMON_H_

	#define _COMMON_H_

	#define TRUE 1

	#define FALSE 0

	#define  SYS(call) ((call) == -1) ?(perror(#call ":ERROR"),exit(1)) :0

	struct joueur {
	    int fd ;
	    char name[256] ;
			int scores[3];
	};
	typedef struct joueur Joueur ;

	enum couleurs {pi, ca, co, tr, pa} ;
	typedef enum couleurs Couleur ;

	struct carte {
		int valeur;
		Couleur couleur;
	};
	typedef struct carte Carte ;

	typedef unsigned short us;

#endif
