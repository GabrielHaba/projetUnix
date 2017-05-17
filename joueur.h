/****************************************
 *	Fichier d'entete de joueur.h
 *	Projet UNIX
 *  Serie 1
 *							Haba, Gabriel
 *					  Nuez Soriano, Diego
 ****************************************/

#include "ipc.h"

#ifndef _JOUEUR_H_

	#define _JOUEUR_H_

	#define SIZE  255

 	#define DESINSCRIPTION -1

	#define SCORES 1

	#define PLI 2

	#define CONTINUER 3

	void err_handler(int);

	void ctrl_c_handler(int);

	void display_welcome();

	void afficher_joueurs(Zone*, int*, int,int);

	void ecarterCartes(Carte**, Carte*, int);

	/*int lecture(int,fd_set);*/

	int lectureChoix();

	int isValidNumber(char*);

	void jouerTour(int, Carte**, int*, Zone*, int*, int,int,int);

	void afficherPli(Zone *,int *,int,int,int);

#endif
