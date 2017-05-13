/****************************************
 *  Fichier serveur.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "common.h"
#include "serveur.h"
#include "socket.h"
#include "shared_memory.h"

static int pret = FALSE;

int main(int argc, char **argv){
    int sck, port, sckJoueur, nbreLu, i, j, nbreFd, keySHData, keySHNbrLecteurs;
    int shmidData, shmidNbrLecteurs, keySEM, setSemId, nbrLecteurs;
    int reponseDesinscription, nbrJoueurs = 0;
    us sem_val_init[2]={1,1};
    us sem_values[2];
    Zone *memoirePtr;
    int *nbrLecteursPtr;
    Zone memoire;
    struct sockaddr_in addr;
    struct sigaction saServer;
    char reponse[SIZE];
    Joueur joueurs[MAX_JOUEURS];
    fd_set sckset;
    fd_set copieset;    /* Permet de faire une copie de sckset */

    /* Verification de l'usage */
    if(argc != 2) {
        fprintf(stderr,"Usage: %s port\n",argv[0]);
        exit(1);
    }
    /* Recuperation du numero de port passe en argument */
    port = atoi(argv[1]);

    /* Demarrage du server */
    sck = initializeServer(&addr, port);


    fprintf(stderr,"Start listening for connections\n");
    /*mettre en mode ecoute le socket : pas bloquant */
    listen(sck,1);
    fprintf(stderr,"\n");

    /* Preparation et armement du signal SIGALARM */
    saServer.sa_handler=handlerAlarm;
    sigemptyset(&saServer.sa_mask);
    saServer.sa_flags = 0;
    SYS(sigaction(SIGALRM,&saServer,NULL));

    FD_SET(sck,&copieset);
    while (nbrJoueurs < MAX_JOUEURS) {
        struct sockaddr_in addr2;
        u_int len2 = sizeof(addr2);
        Joueur joueur;

        if(nbrJoueurs >= 2 && pret){
            break;
        }
        /* remise a zero de la structure */
        FD_ZERO(&sckset);
        /* recuperation de la copie */
        sckset = copieset;
        switch((nbreFd=select(8,&sckset,NULL,NULL,NULL))){
            case -1 :
                if (errno == EINTR) {
                    continue;
                }
                else {
                    printf("%d", nbreFd);
                    perror("Erreur lors du select...\n");
                    exit(4);
                }

            default :
                if(FD_ISSET(sck,&sckset)){
                    if((sckJoueur = accept(sck,(struct sockaddr *)&addr2,&len2))==-1){
                        perror("server accept problem");
                        exit(3);
                    }

                    fprintf(stderr, "Connection from %s %d\n", inet_ntoa(addr2.sin_addr), ntohs(addr2.sin_port));
                    FD_SET(sckJoueur,&copieset);
                    sckset = copieset;
                    /* Recuperation du nom du joueur */
                    if (read(sckJoueur, reponse, sizeof(reponse)) == -1) {
                        perror("Erreur de lecture du nom du joueur...\n");
                        exit(3);
                    }
                    printf("Inscription de %s en cours ...\n", reponse);
                    strcpy(joueur.name, reponse);
                    joueur.fd = sckJoueur;
                    joueurs[nbrJoueurs] = joueur;
                    printf("Inscription reussie !\n");
                    /* Envoie de la reponse */
                    sprintf(reponse, "%s %s\n", INSCRIT, joueur.name);
                    if (write(sckJoueur, reponse, sizeof(reponse)) != sizeof(reponse)) {
                         perror("Erreur d'ecriture de la reponse au client...\n");
                         exit(3);
                    }
                    nbrJoueurs += 1;
                    /* si on vient d'accepter le premier joueur, on lance le timer */
                    if (nbrJoueurs == 1) {
                            alarm(NBR_SEC);
                    }
                }
                else{
                     for (j = 0; j < nbrJoueurs; j++) {
                        if (FD_ISSET(joueurs[j].fd, &sckset)) {
                            if((nbreLu = read(joueurs[j].fd, &reponseDesinscription, sizeof(int))) == -1) {
                                perror("Erreur de lecture lors de demande de desinscription..._\n");
                                exit(3);
                            }
                            if (nbreLu == 0) {
                                //le joueur a fermé le terminal
                            }
                            printf("Desinscription de %s..\n", joueurs[j].name);
                            close(joueurs[j].fd);
                            FD_CLR(joueurs[j].fd, &copieset);
                            modifyTable(joueurs, j, nbrJoueurs);
                            nbrJoueurs--;
                            /* Si on rencontre le joueur recherche, inutile de continuer la boucle */
                            break ;
                        }
                     }
                }
            /* END default */
        } /* END Switch*/


    }
    /* Affichage de la liste des joueurs [TEMPORAIRE] --> permet de vérifier l'inscription */
    fprintf(stdout, "Inscription finie\nIl y a %d joueurs : \n",nbrJoueurs);
    for(i = 0; i < nbrJoueurs; i++){
        printf("\t%s avec le fd %d\n",joueurs[i].name,joueurs[i].fd);
    }

    /***************************************************************/
    /*TEST SEMAPHORE*/
    printf("\nSEMAPHORES\n");
    /*ETAPE 1 -> OBTENTION CLE */
    keySEM = ftok(".",1);
    printf("\nLa cle SEM est %d \n",keySEM);

    /*ETAPE 2 -> CREATION DES SEMAPHORES */
    setSemId = creerSemaphore(keySEM);
    printf("Le setSemId est %d \n",setSemId);

    /*ETAPE 3 -> INITIALISER SEMAPHORES */
    initSemaphore(setSemId,sem_val_init);

    /*ETAPE 4 -> VERIFICATION DES VALEURS SEMAPHORES */
    getValueSems(setSemId,sem_values);
    printf("sem nbrLecteurs : %d, sem data : %d\n",sem_values[0],sem_values[1]);


    /*****************************************************/
    /*TEST MEMOIRE PARTAGEE*/
    printf("\nMEMOIRES PARTAGEES\n");
    /*ETAPE 1 -> OBTENTION CLES POUR LES 2 SH */
    keySHData = ftok(".",2);
    printf("\nLa cle SHData est %d \n",keySHData);

    keySHNbrLecteurs = ftok(".",3);
    printf("La cle SHNbrLecteurs est %d \n",keySHNbrLecteurs);

    /*ETAPE 2 -> CREATION MEMOIRES PARTAGEES*/
    shmidData = creerSharedM(keySHData,sizeof(Zone));
    printf("Le shmidData est %d \n",shmidData);

    shmidNbrLecteurs= creerSharedM(keySHNbrLecteurs,sizeof(int));
    printf("Le shmidNbrLecteurs est %d \n",shmidNbrLecteurs);

    /*ETAPE 3 -> ATTACHEMENT DES MEMOIRES */
    memoirePtr = (Zone *) attacherSharedM(shmidData);

    nbrLecteursPtr = (int *) attacherSharedM(shmidNbrLecteurs);

    /*ETAPE 4 -> INITIALISER NBR DE LECTEURS DANS 2EME MEMOIRE PARTAGEE */
    nbrLecteurs = initNbrLecteurs(nbrLecteursPtr);
    printf("nbrLecteurs initial %d\n",nbrLecteurs);

    /*ETAPE 5 -> ECRITURE DANS MEMOIRE PARTAGEE DE DONNEES */
    ecrireSharedM(memoirePtr,setSemId,1,joueurs);

    /*ETAPE 6 -> LECTURE DANS LA MEMOIRE */
      memoire = lireSharedM(memoirePtr,nbrLecteursPtr,setSemId);
      for(i=0;i<nbrJoueurs;i++){
        printf("Le joueur %s est stocké en SH\n",memoire.joueurs[i].name);
      }


    exit(0);
}

/* Indique que plus de 30 secondes sont passees */
void handlerAlarm(){
    pret = TRUE;
}

/* Retire le joueur a l'indice toRemove de la table */
void modifyTable(Joueur table[MAX_JOUEURS], int toRemove, int taille){
    int i ;
    for (i = toRemove ; i < taille-1 ; i++){
        table[i] = table[i+1] ;
    }
}
