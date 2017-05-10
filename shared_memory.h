/****************************************
 *	Fichier d'entete de shared_memory.h
 *	Projet UNIX
 *  Serie 1
 *							Haba, Gabriel
 *					  Nuez Soriano, Diego
 ****************************************/

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>

#ifndef _SHARED_H_

	#define _SHARED_H_

	key_t key = 9;

 	#define SHMSIZE = 1024;

	#define SHMFLAG = (0644 | IPC_CREATE) ;

	#define CREATE = 0;

	typedef struct record{
		char label[64];
		void *contenu;
	} Record;

	int creerSharedM();

	void * attacherSharedM(int);

	void detacherSharedM(void *);

	void supprimerSharedMemory(int);

	void lireSharedM(int);

	void ecrireSharedM(int);


#endif
