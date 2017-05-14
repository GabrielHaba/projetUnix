/****************************************
 *  Fichier carte.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "carte.h"

void initJeu(Carte** jeu){
  int i ;
  /* Piques */
  for (i = 0 ; i < 10 ; i++){
    Carte c = {i+1, pi};
    (*jeu)[i] = c ;
  }
  /* Carreaux */
  for (i = 0 ; i < 10 ; i++){
    Carte c = {i+1, ca};
    (*jeu)[10+i] = c ;
  }
  /* Coeurs */
  for (i = 0 ; i < 10 ; i++){
    Carte c = {i+1, co};
    (*jeu)[20+i] = c ;
  }
  /* Trèfles */
  for (i = 0 ; i < 10 ; i++){
    Carte c = {i+1, tr};
    (*jeu)[30+i] = c ;
  }
  /* Papayoo */
  for (i = 0 ; i < 20 ; i++){
    Carte c = {i+1, pa};
    (*jeu)[40+i] = c ;
  }
  shuffle(jeu, 60);
}

void afficherCartes(Carte* cartes, int taille) {
  int i ;
  printf("\n=== Ma main ===\n");
  for (i = 0 ; i < taille ; i++){
    printf("\t%d - %d%s\n", i,cartes[i].valeur, lesSymboles[cartes[i].couleur]);
  }
  printf("\n");
}

void shuffle(Carte** cartes, int taille) {
  int i, random ;
  Carte temp ;
  for (i = 0 ; i < taille ; i++){
    random = rand() % 60 ;
    temp = (*cartes)[i] ;
    (*cartes)[i] = (*cartes)[random] ;
    (*cartes)[random] = temp ;
  }
}
