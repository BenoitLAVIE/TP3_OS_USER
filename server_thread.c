#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include "creme.h"

volatile int serveur_en_cours = 0;

void *serveur_udp(void *p)
{
    char *mon_pseudo = (char *)p;
    int sock = preparer_socket_beuip();
    struct ifaddrs *ifaddr, *ifa;
    serveur_en_cours = 1;
    if (getifaddrs(&ifaddr) == -1) 
    {
        struct sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT_BEUIP);
        inet_aton(BROADCAST_ADDR, &dest.sin_addr);
        envoyer_paquet_beuip(sock, &dest, '1', mon_pseudo, NULL);
    }
    else 
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
            {
                struct sockaddr_in *bcast_addr = (struct sockaddr_in *)ifa->ifa_broadaddr;
                if (bcast_addr)
                {
                    char *ip_bcast = inet_ntoa(bcast_addr->sin_addr);
                    if (strcmp(ip_bcast, "0.0.0.0") != 0 && strcmp(ip_bcast, "127.255.255.255") != 0)
                    {
                        struct sockaddr_in dest = {AF_INET, htons(PORT_BEUIP), bcast_addr->sin_addr};
                        envoyer_paquet_beuip(sock, &dest, '1', mon_pseudo, NULL);
                    }
                }
            }
        }
        freeifaddrs(ifaddr);
    }
    struct timeval tv;
    tv.tv_sec = 1; 
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (serveur_en_cours)
    {
        char buf[BUF_SIZE];
        struct sockaddr_in src;
        socklen_t len = sizeof(src);
        int n = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&src, &len);

        if (n > 0 && memcmp(buf + 1, BEUIP_MAGIC, 5) == 0)
        {
            char code = buf[0];
            char *ip_emetteur = inet_ntoa(src.sin_addr);

            if (code == '1' || code == '2')
            {
                supprimeElt(ip_emetteur); 
                ajouteElt(buf + 6, ip_emetteur);
                if (code == '1')
                    envoyer_paquet_beuip(sock, &src, '2', mon_pseudo, NULL);
            }
            else if (code == '0')
            {
                supprimeElt(ip_emetteur);
            }
            else if (code == '9')
            {
                printf("\n[Message] %s\n", buf + 6);
            }
        }
    }
    close(sock);
    return NULL;
}