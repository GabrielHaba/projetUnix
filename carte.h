/****************************************
 *	Fichier d'entete de carte.c
 *	Projet UNIX
 *  Serie 1
 *                    Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

 #include <time.h>
 #include <stdlib.h>
 #include "common.h"


 #ifndef _CARTE_H_

   #define _CARTE_H_

   static char *lesSymboles[] = {"♠", "♦", "♥", "♣", "P"} ;

   void initJeu(Carte**) ;

   void afficherCartes(Carte*, int) ;

   void shuffle(Carte**, int);

 #endif
