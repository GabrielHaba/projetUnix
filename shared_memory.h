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

#ifndef _SHARED_H_

	#define _SHARED_H_

 	#define SHMSIZE 1024

	#define FLAG (0644 | IPC_CREAT)

	#define CREATE 0

	#define NUMSEMNBRLECTEURS 0

	#define NUMSEMECRIT 1

	typedef struct record{
		char label[64];
		void *contenu;
	} Record;

	int creerSharedM(int);

	void * attacherSharedM(int);

	void detacherSharedM(void *);

	void supprimerSharedMemory(int);

	void  *lireSharedM(int,int,int);

	void ecrireSharedM(Zone, int, int, int, Record);

	/****************Semaphore*******************/

	int creerSemaphore(int);

	int up(int,int);

	int down(int,int);


#endif
