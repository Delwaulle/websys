#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <stdio.h>
#ifndef __SOCKET_H__
#define __SOCKET_H__
/** Crée une socket serveur qui écoute sur toute les interfaces IPv4
de la machine sur le port passé en paramètre . La socket retournée
doit pouvoir être utilisée directement par un appel à accept .
La fonction retourne -1 en cas d ’ erreur ou le descripteur de la
socket créée .*/
int creer_serveur(int port );
int afficherMessage(int socket_client);
void traiterClient(int socket_client);
int attendre_socket(int socket_serveur);
#endif

