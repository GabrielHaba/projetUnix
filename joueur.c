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
    Carte ecartee[5] ;
    Couleur couleurPayoo;
    us sem_val_init[2]={1,1};
    us sem_values[2];
    struct sockaddr_in addr;
	  struct hostent *host;
    char reponseServer[256];
    char input[SIZE];
    Zone *memoirePtr;
    int *nbrLecteursPtr;
    Zone memoire;
    int choix;
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

    while ((choix = lectureChoix()) != CONTINUER){
      if (choix == SCORES){
        afficher_joueurs(memoirePtr, nbrLecteursPtr, setSemId);
      }
      else{
        printf("\nAucune carte n'a été jouée à cette manche pour le moment !\n") ;
      }
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
//  jouerTour()


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
	if (strlen(string) == 1){
		return FALSE ;
	}
	for (i = 0; i < strlen(string)-1; i++){
		if (!isdigit(string[i])){
			return FALSE ;
		}
	}
	return TRUE ;
}


int lectureChoix(){
  char buffer[SIZE];
  int nr;
  printf("\n=== ACTIONS ===\n\t1 - Afficher les scores\n\t2 - Consulter le pli en cours\n\t3 - Continuer le jeu\n");
  while (TRUE){
    printf("Votre choix : ");
    fflush(0);
    if ((fgets(buffer, SIZE, stdin) != NULL && isValidNumber(buffer))){
      nr = atoi(buffer);
      if (nr > 0 && nr < 4){
        return nr ;
      }
    }
  }
}

/*
int lecture(int sck,fd_set copieset){
  fd_set fdSet;
  FD_ZERO(&fdSet);
  fdSet=copieset;
  int nbreFd;
  if((nbreFd=select(sck+1,&fdSet,NULL,NULL,NULL))<0){
    perror("Erreur lors de l'ecoute au clavier ou sur le socket ...");
    exit(15);
  }
  if(FD_ISSET(0,&fdSet)){
    char buffer[256];
    int nr;
    if((nr=read(0,&buffer,sizeof(buffer)))<0){
      perror("Erreur de la lecture au clavier ...");
      exit(15);
    }
    if(strncmp(buffer,"SCORES",6)==0){
      return 0;
    }

  }
  return -1;
}*/
