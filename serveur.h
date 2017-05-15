/****************************************
 *	Fichier d'entete de serveur.h
 *	Projet UNIX
 *  Serie 1
 *							Haba, Gabriel
 *					  Nuez Soriano, Diego
 ****************************************/

#include	<arpa/inet.h>

#ifndef _SERVEUR_H_

	#define _SERVEUR_H_

 	#define NBR_SEC 30

	#define SIZE 256

	#define MAX_JOUEURS 4

	#define TAILLE_ECART 5

 	#define INSCRIT "Vous etes inscrit"

	#define DEBUT_PARTIE "La partie va commencer !"

 	void handlerAlarm();

 	void modifyTable(Joueur*, int, int);

	Couleur tiragePapayoo();

#endif