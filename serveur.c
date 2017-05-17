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
#include "ipc.h"
#include "carte.h"

static int pret = FALSE;

int main(int argc, char **argv){
    int sck, port, sckJoueur, nbreLu, i, j,k,l, nbreFd, keySHData, keySHNbrLecteurs;
    int shmidData, shmidNbrLecteurs, keySEM,keySEML, setSemIdData,setSemIdNbrLecteurs, nbrLecteurs;
    int reponseDesinscription, nbreToSend, nbrJoueurs = 0;
    Carte* jeuDeCarte ;
    Carte  ecarts[MAX_JOUEURS][TAILLE_ECART];
    Couleur couleurPayoo;
    us sem_val_init[1]={1};
    us sem_values[1];
    Zone *memoirePtr;
    int *nbrLecteursPtr;
    int premier = 0;
    int nbrCartesPli = 0;
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
    FD_ZERO(&copieset);
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
                for (i = 0; i < NBRE_MANCHE; i++){
                    joueur.scores[i] = 0 ;
                }
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

    /* Suppression de l'Alarme */
    alarm(0);

    /* === Initialisation des sockets et de la memoire partagee ===
    * 1) Semaphores
    * ETAPE 1 --> Obtention de la cle */
    keySEM = ftok(".",1);
    keySEML = ftok(".",4);
    /*ETAPE 2 --> Creation des semaphores */
    setSemIdData = creerSemaphore(keySEM);
    setSemIdNbrLecteurs = creerSemaphore(keySEML);


    /*ETAPE 3 --> Initialisation des semaphores */
    initSemaphore(setSemIdNbrLecteurs,sem_val_init);
    initSemaphore(setSemIdData,sem_val_init);

    /* 2) Memoire partagee
    * ETAPE 1 --> Obtention des cles pour les deux shm */
    keySHData = ftok(".",2);
    keySHNbrLecteurs = ftok(".",3);

    /* ETAPE 2 --> Creation des deux memoires partagees */
    shmidData = creerSharedM(keySHData,sizeof(Zone));
    shmidNbrLecteurs= creerSharedM(keySHNbrLecteurs,sizeof(int));

    /* ETAPE 3 --> Attachement des memoires */
    memoirePtr = (Zone *) attacherSharedM(shmidData);
    nbrLecteursPtr = (int *) attacherSharedM(shmidNbrLecteurs);

    /*ETAPE 4 --> Initialisation nbreJoueurs dans la 2e memoire */
    nbrLecteurs = initNbrLecteurs(nbrLecteursPtr);

    /*ETAPE 5 --> Ecriture des donness dans la shm */
    ecrireSharedM(memoirePtr, setSemIdData, NBRE_JOUEURS, &nbrJoueurs,0);
    ecrireSharedM(memoirePtr, setSemIdData, JOUEURS, joueurs,0);

    /* Allocation de la zone memoire qui va contenir le deck */
    if ((jeuDeCarte = (Carte*)malloc(sizeof(Carte)*60)) == NULL){
        perror("Erreur d'allocation...");
        exit(10);
    }

    /* ===================================================================
    * =                         DEBUT DE PARTIE                         =
    * =================================================================== */

    // Il faut envoyer un message aux joueurs leur disant que la partie commence

    sprintf(reponse, "\n%s\n", DEBUT_PARTIE);
    for (i = 0; i < nbrJoueurs; i++) {
        if (write(joueurs[i].fd, reponse, sizeof(reponse)) != sizeof(reponse)) {
            perror("Erreur lors de l'envoi du message de debut de partie...\n");
            exit(3);
        }
    }
    l=0;

    srand(time(NULL)); /* Permet d'avoir une valeur aleatoire */

    while(l < NBRE_MANCHE){
        /* ============================= */
        /* === Phase de distribution === */
        /* ============================= */

        /* Initialisation du jeu de cartes + melange des cartes */
        initJeu(&jeuDeCarte) ;

        /* On determine la taille des tableaux de cartes à envoyer ...*/
        nbreToSend = 60 / nbrJoueurs ;
        /* Distribution des cartes aux joueurs */
        for (i = 0 ; i < nbrJoueurs ; i++){
            Carte toSend[nbreToSend];
            memcpy(toSend, &jeuDeCarte[nbreToSend*i], nbreToSend * sizeof(Carte));
            if (write(joueurs[i].fd, toSend, sizeof(Carte)*nbreToSend) != sizeof(Carte)*nbreToSend) {
                perror("Erreur lors de l'envoi des cartes...\n");
                exit(77);
            }
        }
        /* ============================= */
        /* === Phase d' écart  === */
        /* ============================= */


        nbreToSend = 5;
        /*Reception des écarts des joueurs*/

        for(i = 0; i < nbrJoueurs ; i++){
            if(read(joueurs[i].fd,ecarts[i], nbreToSend * sizeof(Carte))<0){
                perror("Erreur lors de la reception des cartes écartées par les joueurs\n");
                exit(77);
            }
        }
        /*Envoie des cartes écartées par le joueur i au joueur i+1*/
        for(i=0; i < nbrJoueurs ; i++){
            if (write(joueurs[(i+1)%(nbrJoueurs)].fd,ecarts[i], sizeof(Carte)*nbreToSend) != sizeof(Carte)*nbreToSend) {
                perror("Erreur lors de l'envoi des cartes...\n");
                exit(77);
            }
        }
        //PROBLEME
        ecrireSharedM(memoirePtr,setSemIdData ,NBRE_CARTES_PLI, &nbrCartesPli, 0);

        couleurPayoo = tiragePapayoo();
        /* On previent le joueur de la couleur Papayoo -------> shm? */
        for(i = 0; i < nbrJoueurs ; i++){
            if (write(joueurs[i].fd, &couleurPayoo, sizeof(Couleur)) != sizeof(Couleur)) {
                perror("Erreur lors de l'envoi des cartes...\n");
                exit(77);
            }
        }

        // DEBUT DE LA MANCHE
        // BOUCLE AVEC NOMBRE DE CARTE PAR JOUEUR RESTANTES
        premier = 0;
        //prévenir les joueurs que le premier joueur est ...
        for(k=0; k < NBR_TOURS ; k++){
            for(i = 0; i < nbrJoueurs ; i++){
                if (write(joueurs[i].fd, &premier, sizeof(int)) != sizeof(int)) {
                    perror("Erreur lors de l'envoi de l indice du premier joueur...\n");
                    exit(77);
                }
            }
            deroulementTour(&premier, nbrJoueurs,memoirePtr, joueurs, &setSemIdData,&setSemIdNbrLecteurs,nbrLecteursPtr);
            nbrCartesPli=0;
            ecrireSharedM(memoirePtr,setSemIdData ,NBRE_CARTES_PLI, &nbrCartesPli, 0);

        }
        l++;
    }



    free(jeuDeCarte);
    system("./remove_ipc.sh");  /* Retrait des ipcs */
    SYS(close(sck));
    exit(0);
}

/* ======================================================================= */

/* === Fonctions ===*/


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

Couleur tiragePapayoo(){
    return (int) (rand()/(RAND_MAX+1.0)*4);
}

int determinerPerdant(int nbrJoueurs,Zone *memoirePtr,int *setSemIdData,int *setSemIdNbrLecteurs,int *nbrLecteursPtr,int premier){
    printf("DETERMINERPERDANT\n");
    Zone memoire = lireSharedM(memoirePtr, nbrLecteursPtr, *setSemIdData,*setSemIdNbrLecteurs);
    int i;
    Carte *pli = memoire.pli;
    Joueur *joueurs = memoire.joueurs;
    Couleur couleurPremiereCartePli=(pli[premier]).couleur;

    int perdant=0;
    int valeurMaxCouleurPremiereCartePli=-1;
    for(i=0;i<nbrJoueurs;i++){

        if(pli[i].couleur==couleurPremiereCartePli && pli[i].valeur>valeurMaxCouleurPremiereCartePli ){
            valeurMaxCouleurPremiereCartePli=pli[i].valeur;
            perdant=i;

        }
    }
    return perdant;
}
void deroulementTour(int *numPremierJoueur,int nbrJoueurs,Zone *memoirePtr,Joueur *joueurs, int* setSemIdData,int *setSemIdNbrLecteurs,int *nbrLecteursPtr){
    int j, k, indexJoueur ;
    Carte carteJouee;
    int ton_tour = TON_TOUR;
    int pli_consultable = PLI_CONSULTABLE;
    int avert_perdant = AVERT_PERDANT;
    int fin_tour = FIN_TOUR;
    int perdant = PERDANT;
    int nbrCartesPli=0;
    char nom[SIZE];
    Zone memoire;

    for (j = 0 ; j < nbrJoueurs; j++) {
        indexJoueur = (*numPremierJoueur + j) % nbrJoueurs ;
        // demander au joueur j de jouer une carte
        if (write(joueurs[indexJoueur].fd, &ton_tour, sizeof(int)) != sizeof(int)) {
            perror("Erreur lors de l'envoi de l'invitation à jouer...\n");
            exit(13);
        }
        /* Lire la carte jouee par le joueur j */
        if(read(joueurs[indexJoueur].fd, &carteJouee, sizeof(Carte)) < 0){
            perror("Erreur lors de la reception de la carte jouée...\n");
            exit(13);
        }

        /* Ecrire la carte en mémoire partagée à la position corresponde
        * à celle du joueur */
        ecrireSharedM(memoirePtr, *setSemIdData, CARTES, &carteJouee, indexJoueur);

        nbrCartesPli++;
        /*Ecrire le nbre de cartes qu'il y a dans le pli*/
        ecrireSharedM(memoirePtr, *setSemIdData, NBRE_CARTES_PLI, &nbrCartesPli, indexJoueur);

        /* Avertir tous les joueurs que le pli est consultable */
        for (k = 0; k < nbrJoueurs; k++) {
            if (write(joueurs[k].fd, &pli_consultable, sizeof(int)) != sizeof(int)) {
                perror("Erreur lors de l'envoi de l'avertissement pli consultable...\n");
                exit(13);
            }
        }
    }
    //le perdant devient le premier joueur du tour suivant
    *numPremierJoueur = determinerPerdant(nbrJoueurs,memoirePtr,setSemIdData,setSemIdNbrLecteurs,nbrLecteursPtr,*numPremierJoueur);
    sprintf(nom,"Le perdant du tour est %s\n",joueurs[*numPremierJoueur].name);
    printf("perdant -> premier %d\n",*numPremierJoueur);
    sleep(2);
    //dire aux joueurs qui a perdu le tour
    for(k = 0; k< nbrJoueurs; k++) {
        if (write(joueurs[k].fd, &avert_perdant, sizeof(int)) != sizeof(int)) {
            perror("Erreur lors de l'envoi de l'avertissement du perdant...\n");
            exit(13);
        }
        if (write(joueurs[k].fd,nom, sizeof(char)*(SIZE-1)) != sizeof(char)*(SIZE-1)) {
            perror("Erreur lors de l'envoi du nom du perdant...\n");
            exit(13);
        }

    }
    sleep(2);
    //envoyer le pli en cours au perdant
    printf("numero perdant : %d\n", *numPremierJoueur);
    if (write((joueurs[*numPremierJoueur]).fd,&perdant, sizeof(int)) != sizeof(int)) {
        perror("Erreur lors de l'envoi de l'avertissement au perdant...\n");
        exit(13);
    }
    sleep(1);
    //lecture du pli en cours
    memoire = lireSharedM(memoirePtr, nbrLecteursPtr, *setSemIdData,*setSemIdNbrLecteurs);
    afficherCartes(memoire.pli,nbrJoueurs);

    if (write((joueurs[*numPremierJoueur]).fd, memoire.pli, sizeof(Carte)*nbrJoueurs) != sizeof(Carte)*nbrJoueurs) {
        perror("Erreur lors de l'envoi du pli au perdant...\n");
        exit(13);
    }

    sleep(2);

    // On indique que le tour est terminé...
    for (k = 0; k < nbrJoueurs; k++) {
        if (write(joueurs[k].fd, &fin_tour, sizeof(int)) != sizeof(int)) {
            perror("Erreur lors de l'envoi de l'avertissement de fin de tour...\n");
            exit(13);
        }
    }

}
