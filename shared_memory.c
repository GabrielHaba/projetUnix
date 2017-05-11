/****************************************
 *  Fichier shared_memory.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "shared_memory.h"
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

Joueur *lireSharedM(Zone *addrData,int *addrShmNbrLecteurs, int setSemaphonreId){
  int nbrLecteurs;
  Joueur *tablJoueurs;
  SYSDOWN(down(setSemaphonreId,NUMSEMNBRLECTEURS));
  nbrLecteurs=((*addrShmNbrLecteurs)++);
  if(nbrLecteurs==1){
    SYSDOWN(down(setSemaphonreId,NUMSEMDATA));
  }
  up(setSemaphonreId,NUMSEMNBRLECTEURS);
  tablJoueurs=addrData->joueurs;
  SYSDOWN(down(setSemaphonreId,NUMSEMNBRLECTEURS));
  nbrLecteurs=((*addrShmNbrLecteurs)--);
  if(nbrLecteurs==0){
    up(setSemaphonreId,NUMSEMDATA);
  }
  up(setSemaphonreId,NUMSEMNBRLECTEURS);
  return tablJoueurs;
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
