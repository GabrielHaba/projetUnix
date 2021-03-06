/****************************************
*  Fichier joueur.c
*  Projet UNIX
*  Serie 1
*                          Haba, Gabriel
*                    Nuez Soriano, Diego
****************************************/

#include    "common.h"
#include    "joueur.h"
#include    "socket.h"
#include    "carte.h"


int main(int argc, char** argv) {
    int sck, port, nr, keySem,keySemL, keyShData, keyShNbrLect, setSemIdData, setSemIdNbrLecteurs;
    int shmidData, shmidNbrLecteurs, i, nbrJoueurs, nbrCartes, manche ;
    Carte* mesCartes ;
    Carte ecartee[5] ;
    Couleur couleurPayoo;
    us sem_val_init[1]={1};
    us sem_values[1];
    struct sockaddr_in addr;
    struct hostent *host;
    char reponseServer[256];
    char input[SIZE];
    Zone *memoirePtr;
    int *nbrLecteursPtr;
    Zone memoire;
    int choix;
    int premier;
    /*fd_set fdSet;
    fd_set copieset;*/

    struct sigaction sigCtrlC ;

    signal(SIGPIPE, err_handler);

    sigCtrlC.sa_flags = 0 ;
    sigCtrlC.sa_handler = &ctrl_c_handler ;
    sigemptyset(&sigCtrlC.sa_mask);

    /* Verification de l'usage */
    if (argc != 3) {
        fprintf(stderr,"Usage: %s ip port\n",argv[0]);
        exit(1);
    }

    /* ---------- APPLICATION BODY ---------- */
    display_welcome();
    printf("\nVeuillez entre votre nom de joueur : ");
    fflush(0);
    while ((fgets(input, SIZE, stdin) == NULL) || (strlen(input) <= 1)){
        printf("\nVeuillez entre votre nom de joueur : ");
        fflush(0);
    }
    /* Remplacement du \n par \0 */
    input[strlen(input)-1] = '\0';

    /* Initialisation du socket + connexion au server */
    sck = initializeClient(argv[1], argv[2]);

    /* Envoi du nom du joueur */
    if (write(sck, input, sizeof(input)) != sizeof(input)) {
        perror("\nErreur lors de l'envoi de la reponse...\n");
        exit(3);
    }
    printf("\nInscription en cours...\n");
    /* Confirmation de l'inscription */
    if (read(sck,reponseServer,sizeof(reponseServer)) == -1){
        perror("Erreur de reception de la reponse du serveur...\n");
        exit(3);
    }

    printf("%s\n",reponseServer);

    /* Une fois inscrit, il faut signaler au server
    * toute deconnexion eventuelle pour qu'il
    * désinscrive le joueur */
    SYS(sigaction(SIGINT, &sigCtrlC, NULL)) ;

    if ((nr = read(sck, reponseServer, sizeof(reponseServer))) == -1) {
        if(errno==EINTR){
            int des = -1 ;
            if (write(sck, &des, sizeof(int)) != sizeof(int)) {
                perror("Erreur d'envoi du signal de desinscription\n...");
                exit(3);
            }
            exit(0);
        }
    }
    if(nr == 0){
        printf("Les inscriptions sont terminées ...\n");
    }

    printf("\n%s\n", reponseServer);

    /* === Initialisation des semaphores et de la memoire partagee === */
    /* 1) Semaphores */
    /* ETAPE 1 --> Obtenir une clé pour les semaphores */
    keySem = ftok(".",1);
    keySemL = ftok(".",4);
    /* ETAPE 2 --> Creation des semaphores */
    setSemIdData = creerSemaphore(keySem);
    setSemIdNbrLecteurs = creerSemaphore(keySemL);
    /* ETAPE 3 --> Initialisation des semaphores */
    initSemaphore(setSemIdData, sem_val_init);
    initSemaphore(setSemIdNbrLecteurs, sem_val_init);
    /* 2) Memoire partagee */
    /* ETAPE 1 --> Obtenir une clé pour la SH */
    keyShData = ftok(".",2);
    keyShNbrLect = ftok(".",3);

    /* ETAPE 2 --> Recuperation des memoires partagees */
    shmidData = creerSharedM(keyShData, sizeof(Zone));
    shmidNbrLecteurs= creerSharedM(keyShNbrLect, sizeof(int));

    /*ETAPE 3 --> Attachement des memoires */
    memoirePtr = (Zone *) attacherSharedM(shmidData);

    nbrLecteursPtr = (int *) attacherSharedM(shmidNbrLecteurs);

    /*ETAPE 4 -> Lecture dans la memoire */
    memoire = lireSharedM(memoirePtr,nbrLecteursPtr,setSemIdData,setSemIdNbrLecteurs);
    nbrJoueurs = memoire.nbrJoueurs ;

    afficher_joueurs(memoirePtr, nbrLecteursPtr, setSemIdData,setSemIdNbrLecteurs);

    /* ========================== */
    /* === Debut d'une manche === */
    /* ========================== */
    nbrCartes = 60 / nbrJoueurs ;
    /* Reception des cartes */
    if ((mesCartes = (Carte*)malloc(sizeof(Carte)*nbrCartes)) == NULL){
        perror("Erreur d'allocation...");
        exit(10);
    }


    for (manche = 0; manche < NBRE_MANCHE; manche++){
        /* Réinitialisation du nombre de cartes */
        nbrCartes = 60 / nbrJoueurs ;
        printf("\n\n===== MANCHE %d =====\n", (manche+1));
        /* lecture des cartes sur le socket */
        if (read(sck, mesCartes, sizeof(Carte)*nbrCartes) == -1){
            perror("Erreur de reception des cartes...\n");
            exit(3);
        }

        /* Ecart des cartes */
        ecarterCartes(&mesCartes, (Carte*)ecartee, nbrCartes);
        /* Affichage des cartes à ecarter */
        printf("Cartes envoyées :\n");
        afficherCartes(ecartee, A_ECARTER);
        /* Envoi des cartes au serveur */
        if (write(sck, ecartee, sizeof(Carte)*A_ECARTER) != sizeof(Carte)*A_ECARTER){
            perror("Erreur d'envoi des cartes a ecarter au serveur...\n");
            exit(3);
        }
        /* Recuperation des cartes du joueur suivant */
        if (read(sck, ecartee, sizeof(Carte)*A_ECARTER) == -1){
            perror("Erreur de reception des cartes...\n");
            exit(3);
        }
        printf("Cartes reçues :\n");
        afficherCartes(ecartee, 5);
        for (i = 0; i < A_ECARTER; i++){
            mesCartes[nbrCartes-5+i] = ecartee[i];
        }
        afficherCartes(mesCartes, nbrCartes);

        /* Lecture de la couleur du Papayoo */
        if (read(sck, &couleurPayoo, sizeof(Couleur)) == -1){
            perror("Erreur de reception des cartes...\n");
            exit(3);
        }
        printf("\nLa couleur du Papayoo est le %s !\n", lesSymboles[couleurPayoo]);

        // DEBUT DE LA MANCHE
        // BOUCLE AVEC NOMBRE DE CARTE
        /*Lecture du premier joueur */
        int taille_mes_plis = 0, x;
        Carte* mesPlis;

        if ((mesPlis = (Carte*)malloc(sizeof(Carte)*60)) == NULL){
            perror("Erreur d'allocation de la mémoire\n");
        }

        for (x = 0; x < NBR_TOURS; x++){
            if (read(sck, &premier, sizeof(int)) == -1){
                perror("Erreur de reception de l indice du premier joueur...\n");
                exit(3);
            }
            jouerTour(sck, &mesCartes, &nbrCartes, memoirePtr, nbrLecteursPtr, setSemIdData,setSemIdNbrLecteurs, premier,(Carte *) mesPlis,&taille_mes_plis,nbrJoueurs);
        }
    }

    exit(0);
}

/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */

/* === Fonctions ===*/

void err_handler(int unused){
    fprintf(stderr,"Connection broken !\n");
    return;
}

void ctrl_c_handler(int signal){
    printf("\nDesinscription ...\n");
    return;
}

void display_welcome(){
    printf("\n======================================\n");
    printf("=              PAPA-YOO              =\n");
    printf("=            -- Online --            =\n");
    printf("======================================\n");
}

void afficher_joueurs(Zone* memoirePtr, int* nbrLecteursPtr, int setSemIdData,int setSemIdNbrLecteurs){
    Zone memoire ;
    int nbrJoueurs, i;
    memoire = lireSharedM(memoirePtr, nbrLecteursPtr, setSemIdData,setSemIdNbrLecteurs);
    nbrJoueurs = memoire.nbrJoueurs ;
    printf("\n\n=== Tableau des scores ===\n\n");
    printf("Joueurs\t\tmanche 1\tmanche 2\tmanche 3\n");
    printf("--------------------------------------------------------\n");
    for (i = 0; i < memoire.nbrJoueurs; i++) {
        printf("%s\t\t%d\t\t%d\t\t%d\n", memoire.joueurs[i].name, memoire.joueurs[i].scores[0], memoire.joueurs[i].scores[1], memoire.joueurs[i].scores[2]);
    }
}

void ecarterCartes(Carte** mesCartes, Carte* ecartees, int nbrCartes){
    int i, index ;
    char input[5] ;
    printf("=== Ecart des cartes ===\n");
    for (i = 0; i < A_ECARTER; i++){
        printf("\n=== Ma main ===\n");
        afficherCartes(*mesCartes, nbrCartes);
        printf("\nEntrez le numéro de la carte à écarter : ");
        fflush(0);
        while(TRUE){
            if((fgets(input, 5, stdin) != NULL) && isValidNumber(input)){
                index = atoi(input);
                if (index < nbrCartes && index >= 0) {
                    break ;
                }
            }
            printf("\nEntrez le numéro de la carte à écarter : ");
            fflush(0) ;
        }
        /* On place la carte dans le tableau */
        ecartees[i] = (*mesCartes)[index] ;
        (*mesCartes)[index] = (*mesCartes)[nbrCartes-1];
        nbrCartes-- ;
    }
}

int isValidNumber(char* string){
    int i ;
    /* Si il n'y a eu qu'un \n d'entré... */
    if (string[0]=='\n'){
        return FALSE ;
    }
    for (i = 0; i < strlen(string)-1; i++){
        if (!isdigit(string[i])){
            return FALSE ;
        }
    }
    return TRUE ;
}




void jouerTour(int sck, Carte** mesCartes, int* nbrCartes, Zone* memoirePtr, int* nbrLecteursPtr, int setSemIdData,int setSemIdNbrLecteurs,int premier,Carte *mesPlis,int *taille_mes_plis,int nbrJoueurs){
    fd_set fdSet, copieset ;
    int nbreFd, action, index, toDo ;
    Carte carte ;
    char input[SIZE] ;
    char perdant[SIZE];
    /* Initialisation des fdset*/
    FD_ZERO(&copieset) ;
    FD_SET(sck, &copieset);
    FD_SET(STDIN, &copieset);

    while (TRUE){
        FD_ZERO(&fdSet);
        fdSet = copieset ;
        printf("C'est bientôt votre tour...\n");
        printf("\n=== ACTIONS ===\n\t1 - Afficher les scores\n\t2 - Consulter le pli en cours\n");
        printf("Votre choix : ");
        //  fflush(0);
        switch((nbreFd=select(sck+1, &fdSet, NULL, NULL, NULL))){
            case -1 :
            perror("Erreur du select...\n");
            exit(4);

            default :
            /* === CAS OU L'UTILISATEUR ENTRE QUELQUE CHOSE AU CLAVIER === */
            if (FD_ISSET(STDIN, &fdSet)){
                int nr;
                if ((nr=read(STDIN, input, sizeof(char)*SIZE)) < 0){
                    perror("Erreur de lecture de l'action...\n");
                }


                input[nr-1]='\0';

                if (!isValidNumber(input)){
                    printf("Action inconnue...\n");
                    continue ;
                }
                toDo = atoi(input);
                if (toDo == SCORES) {
                    afficher_joueurs(memoirePtr, nbrLecteursPtr, setSemIdData,setSemIdNbrLecteurs);
                }
                else { // Afficher le pli
                    afficherPli(memoirePtr,nbrLecteursPtr,setSemIdData,setSemIdNbrLecteurs,premier);
                }
            } /* Fin input joueur */
            /* CAS OU LE SERVEUR SIGNALE AU JOUEUR QUE C'EST SON TOUR === */
            else {
                FD_ZERO(&fdSet);
                fdSet = copieset ;
                printf("je recois une action du serveur\n");

                if (read(sck, &action, sizeof(int)) < 0){
                    perror("Erreur de lecture de action !\n");
                    exit(8);
                }
                printf("\n\n\n============ L'ACTION RECUE EST %d =============\n\n\n", action);
                if (action == TON_TOUR) {
                    printf("\nC'est à votre tour de joueur !\n");
                    printf("\n=== Ma main ===\n");
                    afficherCartes(*mesCartes, *nbrCartes);
                    printf("Pli courant : \n");
                    afficherPli(memoirePtr,nbrLecteursPtr,setSemIdData,setSemIdNbrLecteurs,premier);
                    /* Lecture du numero de la carte a envoyer */
                    while(TRUE){
                        printf("\nChoisissez une carte à joueur : ");
                        fflush(0);
                        if((fgets(input, SIZE, stdin) != NULL) && isValidNumber(input)){
                            index = atoi(input);
                            if (index < (*nbrCartes) && index >= 0) {
                                break ;
                            }
                        }
                    }
                    /* L'index est valide, et on récupère et on envoie la carte */
                    carte = (*mesCartes)[index] ;
                    if (write(sck, &carte, sizeof(Carte)) != sizeof(Carte)){
                        perror("Erreur d'envoi de la carte jouée...\n");
                        exit(4);
                    }
                    (*mesCartes)[index] = (*mesCartes)[*(nbrCartes)-1];
                    (*nbrCartes) -= 1 ;
                } /*FIN TON_TOUR*/
                else if (action == PLI_CONSULTABLE){
                    printf("Le pli a été mis à jour et est consultable\n");
                    fflush(0);
                }
                else if (action == AVERT_PERDANT){
                    // Faire read du nom du perdant, plus afficher
                    if (read(sck, perdant, sizeof(char)*SIZE) < 0) {
                        perror("Erreur de lecture du perdant...\n");
                        exit(7);
                    }
                    printf("%s\n", perdant);
                }
                else if (action == PERDANT) {
                    //Carte test[nbrJoueurs];
                    printf("Attente du pli...\n");
                    if (read(sck, (mesPlis+(*taille_mes_plis)), sizeof(Carte)*nbrJoueurs) != sizeof(Carte)*nbrJoueurs){
                        perror("Erreur de lecture du pli...\n");
                        exit(7);
                    }
                    printf("\n\nTAILLE DE MES PLIS : %d\n", (*taille_mes_plis));
                    (*taille_mes_plis) += nbrJoueurs ;
                    afficherCartes(mesPlis, *taille_mes_plis);
                    printf("\n\nTAILLE DE MES PLIS : %d\n", (*taille_mes_plis));
                }
                else { /* --> Fin du tour */
                    printf("FIN du tour\n");
                    fflush(0);
                    return ;
                }
            }
        }
    }
}
void afficherPli(Zone *memoirePtr,int *nbrLecteursPtr,int setSemaphonreIdData,int setSemIdNbrLecteurs, int premier){
    Zone memoire = lireSharedM(memoirePtr,nbrLecteursPtr,setSemaphonreIdData,setSemIdNbrLecteurs);
    Joueur *joueurs = memoire.joueurs;
    Carte *pli = memoire.pli;
    int nbrCartesPli = memoire.nbrCartesPli;
    int nbrJoueurs = memoire.nbrJoueurs;
    int i ;
    printf("premier est %d\n",premier);
    printf("nbr cartes pli %d\n",nbrCartesPli);
    printf("\n");
    if(nbrCartesPli==0){
        printf("Le pli est vide pour le moment ! \n");
    }
    for (i = premier ; i < premier + nbrCartesPli ; i++){
        printf("\t%s - %d%s\n", joueurs[i%nbrJoueurs].name,pli[i%nbrJoueurs].valeur, lesSymboles[pli[i%nbrJoueurs].couleur]);
    }
    printf("\n");
}
