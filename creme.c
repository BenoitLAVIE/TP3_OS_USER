#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "creme.h"

struct elt *liste_contacts = NULL;
pthread_mutex_t mutex_table = PTHREAD_MUTEX_INITIALIZER;

void ajouteElt(char *pseudo, char *adip)
{
    pthread_mutex_lock(&mutex_table);
    struct elt *nouveau = malloc(sizeof(struct elt));
    if (!nouveau)
        return;
    memset(nouveau->nom, 0, LPSEUDO + 1);
    strncpy(nouveau->nom, pseudo, LPSEUDO);
    memset(nouveau->adip, 0, 16);
    strncpy(nouveau->adip, adip, 15);
    nouveau->next = NULL;

    struct elt **curr = &liste_contacts;
    while (*curr && strcmp((*curr)->nom, pseudo) < 0)
    {
        curr = &((*curr)->next);
    }
    nouveau->next = *curr;
    *curr = nouveau;
    pthread_mutex_unlock(&mutex_table);
}

void supprimeElt(char *adip)
{
    pthread_mutex_lock(&mutex_table);
    struct elt **curr = &liste_contacts, *temp;
    while (*curr)
    {
        if (strcmp((*curr)->adip, adip) == 0)
        {
            temp = *curr;
            *curr = (*curr)->next;
            free(temp);
            pthread_mutex_unlock(&mutex_table);
            return;
        }
        curr = &((*curr)->next);
    }
    pthread_mutex_unlock(&mutex_table);
}

void listeElts()
{   
    pthread_mutex_lock(&mutex_table);
    struct elt *curr = liste_contacts;
    while (curr)
    {
        printf("%s : %s\n", curr->adip, curr->nom);
        curr = curr->next;
    }
    pthread_mutex_unlock(&mutex_table);
}

void viderListe() {
    pthread_mutex_lock(&mutex_table);
    struct elt *curr = liste_contacts;
    while (curr) {
        struct elt *temp = curr;
        curr = curr->next;
        free(temp);
    }
    liste_contacts = NULL;
    pthread_mutex_unlock(&mutex_table);
}

int preparer_socket_beuip()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int broadcast_perm = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_perm, sizeof(broadcast_perm));

    struct sockaddr_in serv_addr = {AF_INET, htons(PORT_BEUIP), {INADDR_ANY}};
    bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    return sock;
}

void envoyer_paquet_beuip(int sock, struct sockaddr_in *dest, char code, char *p1, char *p2)
{
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    buffer[0] = code;
    memcpy(buffer + 1, BEUIP_MAGIC, 5);
    int offset = 6;
    if (p1)
    {
        strcpy(buffer + offset, p1);
        offset += strlen(p1) + 1;
    }
    if (p2)
    {
        strcpy(buffer + offset, p2);
    }
    sendto(sock, buffer, BUF_SIZE, 0, (struct sockaddr *)dest, sizeof(*dest));
}