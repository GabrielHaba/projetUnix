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
    int sck, port, nr, keySem, keyShData, keyShNbrLect, setSemId;
    int shmidData, shmidNbrLecteurs, i, nbrJoueurs, nbrCartes ;
    Carte* mesCartes ;
    us sem_val_init[2]={1,1};
    us sem_values[2];
    struct sockaddr_in addr;
	struct hostent *host;
    char reponseServer[256];
    char input[SIZE];
    Zone *memoirePtr;
    int *nbrLecteursPtr;
    Zone memoire;

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

    /* ETAPE 2 --> Creation des semaphores */
    setSemId = creerSemaphore(keySem);

    /* ETAPE 3 --> Initialisation des semaphores */
    initSemaphore(setSemId, sem_val_init);

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
    memoire = lireSharedM(memoirePtr,nbrLecteursPtr,setSemId);
    nbrJoueurs = memoire.nbrJoueurs ;

    afficher_joueurs(memoirePtr, nbrLecteursPtr, setSemId);

    /* ========================== */
    /* === Debut d'une manche === */
    /* ========================== */

    nbrCartes = 60 / nbrJoueurs ;
    /* Reception des cartes */
    if ((mesCartes = (Carte*)malloc(sizeof(Carte)*nbrCartes)) == NULL){
      perror("Erreur d'allocation...");
      exit(10);
    }
    /* lecture des cartes sur le socket */
    if (read(sck, mesCartes, sizeof(Carte)*nbrCartes) == -1){
        perror("Erreur de reception des cartes...\n");
        exit(3);
    }
    afficherCartes(mesCartes, nbrCartes);



 exit(0);
}

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

void afficher_joueurs(Zone* memoirePtr, int* nbrLecteursPtr, int setSemId){
	Zone memoire ;
	int nbrJoueurs, i;
	memoire = lireSharedM(memoirePtr, nbrLecteursPtr, setSemId);
	nbrJoueurs = memoire.nbrJoueurs ;
	printf("\n\n=== Tableau des scores ===\n\n");
	printf("Joueurs\t\tmanche 1\tmanche 2\tmanche 3\n");
	printf("--------------------------------------------------------\n");
	for (i = 0; i < memoire.nbrJoueurs; i++) {
        printf("%s\t\t%d\t\t%d\t\t%d\n", memoire.joueurs[i].name, memoire.joueurs[i].scores[0], memoire.joueurs[i].scores[1], memoire.joueurs[i].scores[2]);
    }


}
