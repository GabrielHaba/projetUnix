/****************************************
 *	Fichier d'entete de shared_memory.h
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

	#define  SYSDOWN(call) ((call) == -1) ? printf("Veuillez attendre qq instants : ces données ne sont pas disponible pour le moment\n") :0

	#define _SHARED_H_

	#define FLAG (0644 | IPC_CREAT)

	#define CREATE 0

	#define NUMSEMNBRLECTEURS 0

	#define NUMSEMDATA 1

	typedef struct zone{
		Joueur joueurs[4];
	} Zone;

	int creerSharedM(int,int);

	void * attacherSharedM(int);

	void detacherSharedM(void *);

	void supprimerSharedMemory(int);

	Joueur *lireSharedM(Zone *,int *, int);

	void ecrireSharedM(Zone*, int, Joueur*);


	/****************Semaphore*******************/

	int creerSemaphore(int);

	int up(int,int);

	int down(int,int);


#endif
