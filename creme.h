#ifndef CREME_H
#define CREME_H

#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>

#define PORT_BEUIP 9998
#define BEUIP_MAGIC "BEUIP"
#define BUF_SIZE 512
#define LPSEUDO 23
#define BROADCAST_ADDR "192.168.88.255"

extern volatile int serveur_en_cours; 
extern struct elt *liste_contacts;
extern pthread_mutex_t mutex_table;

struct elt
{
    char nom[LPSEUDO + 1];
    char adip[16];
    struct elt *next;
};

extern struct elt *liste_contacts;
extern pthread_mutex_t mutex_table;

void ajouteElt(char *pseudo, char *adip);
void supprimeElt(char *adip);
void listeElts(void);
void viderListe(void);

int preparer_socket_beuip(void);
void envoyer_paquet_beuip(int sock, struct sockaddr_in *dest, char code, char *p1, char *p2);
void *serveur_udp(void *p);

#endif