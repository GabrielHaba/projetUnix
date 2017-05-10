/****************************************
 *  Fichier shared_memory.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "shared_memory.h"


int creerSharedM(){
  int shmid=-1;
  if((shmid=shmget(key,SHMSIZE,SHMFLAG))<0){
    perror("Erreur lors de la création de la mémoire partagée ! ");
    exit(6);
  }
  return shmid;
}

void *attacherSharedM(int shmid){
  void *addr=NULL;

  if ((addr=shmat(shmid,NULL,0)<0){
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

  if(shmctl(shmid,SHM_RMID,NULL)<0){
    perror("Erreur lors de la suppression du segment de mémoire partagée !");
    exit(6);
  }
}
