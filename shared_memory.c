/****************************************
 *  Fichier shared_memory.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "shared_memory.h"
#include <stdio.h>


int creerSharedM(int key){
  int shmid=-1;
  if((shmid=shmget(key,SHMSIZE,FLAG))<0){
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

void *lireSharedM(int shmid,int seSemaphonreId){
    /*if(down(setSemaphonreId,))*/
}

void ecrireSharedM(Zone* shmAddr, int semaphores, Joueur[4] toWrite) {
  Joueur* ptr ;
  if (down(semaphores, NUMSEMDATA) < 0) {
      printf("La mémoire n'est pas disponible pour le moment...\n");
      return ;
  }
  /* Ecrire les donnees */
  for (ptr = toWrite; ptr-toWrite < 4; ptr++) {
    (*shmAddr) = *ptr ;
    shmAddr++ ;
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
