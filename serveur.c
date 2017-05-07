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

static int pret = FALSE;

int main(int argc, char **argv){
    int sck, port, sckJoueur, nbreLu, i, j, nbreFd;
    int reponseDesinscription, nbrJoueurs = 0;
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
