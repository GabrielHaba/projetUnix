/****************************************
 *  Fichier ipc.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "ipc.h"
#include <stdio.h>


int creerSharedM(int key,int taille){
  int shmid=-1;
  if((shmid=shmget(key,taille,FLAG))<0){
    perror("Erreur lors de la création de la mémoire partagée ! ");
    exit(6);
  }
  return shmid;
}

void *attacherSharedM(int shmid){
  void *addr=NULL;

  if ((addr=shmat(shmid,NULL,0))<0){
    perror("Erreur lors du rattachement du segment à l'espace de données du processus");
    exit(6);
  }

  return addr;
}

void detacherSharedM(void *addr){
  if(shmdt(addr)<0){
    perror("Erreur lors du détachement du segment de mémoire partagée !");
    exit(6);
  }
}
void supprimerSharedMemory(int shmid){

  if(shmctl(shmid,IPC_RMID,NULL)<0){
    perror("Erreur lors de la suppression du segment de mémoire partagée !");
    exit(6);
  }
}
int initNbrLecteurs(int *addrShmNbrLecteurs){
  *addrShmNbrLecteurs=0;
  return *addrShmNbrLecteurs;
}


Zone lireSharedM(Zone *addrData, int *addrShmNbrLecteurs, int setSemaphonreId){
  int nbrLecteurs;
  Zone memoire;
  SYSDOWN(down(setSemaphonreId,NUMSEMNBRLECTEURS));
  nbrLecteurs=((*addrShmNbrLecteurs)++);
  if(nbrLecteurs==1){
    SYSDOWN(down(setSemaphonreId,NUMSEMDATA));
  }
  up(setSemaphonreId,NUMSEMNBRLECTEURS);
  memoire=*addrData;
  SYSDOWN(down(setSemaphonreId,NUMSEMNBRLECTEURS));
  nbrLecteurs=((*addrShmNbrLecteurs)--);
  if(nbrLecteurs==0){
    up(setSemaphonreId,NUMSEMDATA);
  }
  up(setSemaphonreId,NUMSEMNBRLECTEURS);
  return memoire;
}

void ecrireSharedM(Zone* shmAddr, int semaphores,int cas, void* toWrite,int positionEcriture) {

  int i = positionEcriture ;
  if (down(semaphores, NUMSEMDATA) < 0) {
      printf("La mémoire n'est pas disponible pour le moment...\n");
      return ;
  }
  /* Ecrire les donnees */
  if (cas == NBRE_JOUEURS) {
    shmAddr->nbrJoueurs = *((int*)toWrite) ;
  }
  else if (cas == JOUEURS) {
    Joueur *ptr;
    for (ptr =(Joueur *) toWrite; ptr- ((Joueur *) toWrite) < 4; ptr++) {
      (shmAddr->joueurs)[i] = *ptr ;
      i += 1 ;
    }
  }
  else if (cas == CARTES) {
    (shmAddr->pli)[i] = *((Carte*) toWrite);
  }
  else {
    printf("Action inconnue...\n");
    return  ;
  }

  if (up(semaphores, NUMSEMDATA) < 0) {
      printf("La mémoire n'est pas disponible pour le moment...\n");
      return ;
  }


}

/***************************SEMAPHORE*************************************/
int creerSemaphore(int key){
  int setSemId;
  if((setSemId=semget(key,2,FLAG ))<0){
    perror("Erreur lors de la création de l'ensemble de sémaphore");
    exit(7);
  }
  return setSemId;
}
/*us correspond a unsigned short -> voir common.h*/
void initSemaphore(int setSemId, us *sem_val_init){
  if( semctl(setSemId,0,SETALL,sem_val_init) < 0 ){
    	perror("Erreur lors de l'initialisation des semaphores");
		  exit(7);
	}
}
void getValueSems(int setSemId, us *values){
  if( semctl(setSemId,0,GETALL,values) < 0 ){
     perror("Erreur lors de la récuprération des valeurs des semaphores");
     exit(7);
 }
}

int up(int setId,int numSemaphore){
  struct sembuf operation={numSemaphore,1,IPC_NOWAIT};
  struct sembuf arrayOperations[1]={operation};
  return semop(setId,arrayOperations,1);
}

int down(int setId,int numSemaphore){
  struct sembuf operation={numSemaphore,-1,IPC_NOWAIT};
  struct sembuf arrayOperations[1]={operation};
  return semop(setId,arrayOperations,1);
}
