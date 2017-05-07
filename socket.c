/****************************************
 *  Fichier socket.c
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/

#include "socket.h"

int initializeServer(struct sockaddr_in* addr, int port){
	int sck; /* File descriptor du socket */

	/* AF_INET indique qu'on utilise une adresse IPv4 
	 * SOCK_STREAM indique qu'on utilise le protocol
	 * TCP. L'instruction socket renvoie un file descriptor
	 */
	if ((sck = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("Erreur d'ouverture du socket... [SERVER]\n");
    	exit(1);
	}

	bzero((char*)addr,sizeof(struct sockaddr_in));
    addr->sin_family      = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port        = htons(port);

	if (bind(sck, (struct sockaddr *)addr, sizeof(*addr)) < 0 ){
	    perror("Erreur lors du bin...\n");
	    exit(1);
	}
	return sck ;
}

int initializeClient( char* hostname, char* portString){
	int sck, port ;
	struct sockaddr_in addr ;
	struct hostent *host;

	if ((sck = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server - socket");
        exit(1);
    }

    host = gethostbyname(hostname);
    port = atoi(portString);

    if(host == NULL) {
		fprintf(stderr,"Host inconnu...\n");
		exit(1);
	}

	bzero((char*)&addr,sizeof(struct sockaddr_in));
    addr.sin_family      = AF_INET;
	bcopy(host->h_addr,(char*)&addr.sin_addr.s_addr,host->h_length);
    addr.sin_port        = htons(port);

    if (connect(sck, (struct sockaddr *)&addr, sizeof((addr))) < 0) {
        perror("Erreur de connexion au serveur...\n");
        exit(1);
    }

    return sck ;
}
