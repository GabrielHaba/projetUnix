/****************************************
 *	Fichier d'entete de ipc.h
 *	Projet UNIX
 *  Serie 1
 *							Haba, Gabriel
 *					  Nuez Soriano, Diego
 ****************************************/

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include "common.h"

#ifndef _SHARED_H_

	#define  SYSDOWN(call) ((call) == -1) ? printf("Veuillez attendre qq instants : ces données ne sont pas disponibles pour le moment\n") :0

	#define _SHARED_H_

	#define FLAG (0644 | IPC_CREAT)

	#define CREATE 0

	#define NUMSEMNBRLECTEURS 0

	#define NUMSEMDATA 1

	/* Permet de savoir ce qu'on ecrit */
	#define NBRE_JOUEURS 0

	#define JOUEURS 1

	#define CARTES 2

	#define NBRE_CARTES_PLI 3

	typedef struct zone{
		int nbrJoueurs ;
		Joueur joueurs[4];
		int nbrCartesPli;
		Carte pli[4];
	} Zone;

	int creerSharedM(int,int);

	void * attacherSharedM(int);

	void detacherSharedM(void *);

	void supprimerSharedMemory(int);

	int initNbrLecteurs(int *);

	Zone lireSharedM(Zone *,int *, int, int);

	void ecrireSharedM(Zone*, int,int,void *,int);


	/****************Semaphore*******************/

	int creerSemaphore(int);

	void getValueSems(int, us *);

	void initSemaphore(int, us *);

	int up(int,int);

	int down(int,int);


#endif
