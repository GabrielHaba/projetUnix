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

	void err_handler(int);

	void ctrl_c_handler(int);

	void display_welcome();

	void afficher_joueurs(Zone*, int*, int);

	void ecarterCartes(Carte**, Carte*, int);

	int isValidNumber(char*);

#endif
