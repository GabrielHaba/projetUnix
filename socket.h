/****************************************
 *  Fichier d'entete de socket.h
 *  Projet UNIX
 *  Serie 1
 *                          Haba, Gabriel
 *                    Nuez Soriano, Diego
 ****************************************/
#include "common.h"

#ifndef _SOCKET_H_

	#define _SOCKET_H_

	int initializeServer(struct sockaddr_in* , int);

	int initializeClient( char*, char*);

#endif
