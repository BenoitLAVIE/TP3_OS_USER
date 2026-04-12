#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "creme.h"

#define NBMAXC 20
#define HIST_FILE ".biceps_history"

static char *Mots[100];
static int NMots;
pthread_t tid_udp;
int serveur_actif = 0;
char *mon_pseudo_global = NULL;

typedef struct
{
    char *nom;
    int (*fonction)(int, char **);
} ComInt;

static ComInt TabCom[NBMAXC];
static int NbCom = 0;

void ajouteCom(char *n, int (*f)(int, char **))
{
    if (NbCom < NBMAXC)
    {
        TabCom[NbCom].nom = n;
        TabCom[NbCom++].fonction = f;
    }
}

void commande(char octet1, char *message, char *pseudo_cible)
{
    if (octet1 == '3')
    {
        listeElts();
    }
    else if (octet1 == '4' || octet1 == '5')
    {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        struct elt *curr = liste_contacts;
        while (curr)
        {
            if (octet1 == '5' || (pseudo_cible && strcmp(curr->nom, pseudo_cible) == 0))
            {
                struct sockaddr_in dest;
                dest.sin_family = AF_INET;
                dest.sin_port = htons(PORT_BEUIP);
                inet_aton(curr->adip, &dest.sin_addr);
                envoyer_paquet_beuip(sock, &dest, '9', message, NULL);
                if (octet1 == '4')
                    break;
            }
            curr = curr->next;
        }
        close(sock);
    }
}

int Sortie(int n, char **p) { 
    exit(0); 
    }

int ChangeDir(int n, char **p)
{
    if (n > 1)
        chdir(p[1]);
    return 1;
}

int PrintDir(int n, char **p)
{
    char b[1024];
    getcwd(b, 1024);
    printf("%s\n", b);
    return 1;
}

int Version(int n, char **p)
{
    printf("biceps version 3.0\n");
    return 1;
}

int cat(int n, char **p)
{
    if (n < 2)
    {
        fprintf(stderr, "Usage: cat <filename>\n");
        return 1;
    }
    FILE *f = fopen(p[1], "r");
    if (!f)
    {
        perror("cat");
        return 1;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f))
        printf("%s", buffer);
    fclose(f);
    printf("\n");
    return 1;
}

int ps(int n, char **p)
{
    system("ps aux");
    return 1;
}

int ls(int n, char **p)
{
    system("ls -l");
    return 1;
}

int echo(int n, char **p)
{
    for (int i = 1; i < n; i++)
        printf("%s ", p[i]);
    printf("\n");
    return 1;
}

int help(int n, char **p)
{
    printf("Commandes internes disponibles :\n");
    for (int i = 0; i < NbCom; i++)
        printf("- %s\n", TabCom[i].nom);
    return 1;
}

int lancer_serveur_beuip(int n, char **p)
{
    if (serveur_actif)
    {
        printf("Serveur déjà actif\n");
        return 1;
    }
    if (n < 2)
    {
        fprintf(stderr, "Usage: beuip start <pseudo>\n");
        return 1;
    }
    mon_pseudo_global = strdup(p[1]);
    if (pthread_create(&tid_udp, NULL, serveur_udp, (void *)mon_pseudo_global) != 0)
    {
        perror("pthread_create");
        free(mon_pseudo_global);
        return 1;
    }
    serveur_actif = 1;
    printf("Serveur thread lancé ID interne : %lu\n", (unsigned long)tid_udp);
    return 1;
}

int stop_serveur_beuip(int n, char **p) {
    if (!serveur_actif) return 1;
    serveur_en_cours = 0; 

    pthread_mutex_lock(&mutex_table);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct elt *curr = liste_contacts;
    while (curr) {
        struct sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT_BEUIP);
        inet_aton(curr->adip, &dest.sin_addr);
        envoyer_paquet_beuip(sock, &dest, '0', mon_pseudo_global, NULL);
        curr = curr->next;
    }
    close(sock);
    pthread_mutex_unlock(&mutex_table);
    pthread_join(tid_udp, NULL);
    viderListe(); 
    free(mon_pseudo_global);
    mon_pseudo_global = NULL;
    serveur_actif = 0;

    printf("Serveur arrêté\n");
    return 1;
}

int cmd_beuip(int n, char **p) {
    if (n < 2) {
        printf("Usage: beuip <start|stop|list|message>\n");
        return 1;
    }

    if (strcmp(p[1], "start") == 0) {
        return lancer_serveur_beuip(n - 1, &p[1]);
    } 
    else if (strcmp(p[1], "stop") == 0) {
        return stop_serveur_beuip(n - 1, &p[1]);
    } 
    else if (strcmp(p[1], "list") == 0) {
        commande('3',NULL,NULL);
        return 1;
    } 
    else if (strcmp(p[1], "message") == 0) {
        if (n < 4) {
            printf("Usage: beuip message <user|all> <texte>\n");
            return 1;
        }
        char message_complet[256] = "";
        for (int i = 3; i < n; i++) {
            strcat(message_complet, p[i]);
            if (i < n - 1) strcat(message_complet, " ");
        }

        if (strcmp(p[2], "all") == 0) {
            commande('5', message_complet, NULL);
        } else {
            commande('4', message_complet, p[2]);
        }
        return 1;
    }

    printf("commande beuip inconnue : %s\n", p[1]);
    return 1;
}

void majComInt()
{
    ajouteCom("exit", Sortie);
    ajouteCom("cd", ChangeDir);
    ajouteCom("pwd", PrintDir);
    ajouteCom("vers", Version);
    ajouteCom("cat", cat);
    ajouteCom("ps", ps);
    ajouteCom("ls", ls);
    ajouteCom("echo", echo);
    ajouteCom("beuip", cmd_beuip);
    ajouteCom("help", help);
}

int execComInt(int n, char **p)
{
    for (int i = 0; i < NbCom; i++)
    {
        if (strcmp(p[0], TabCom[i].nom) == 0)
        {
            TabCom[i].fonction(n, p);
            return 1;
        }
    }
    return 0;
}

void execComExt(char **p)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(p[0], p);
        perror("biceps");
        exit(1);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

int analyseCom(char *b) {
    NMots = 0;
    if (!b) return 0;

    char *token = strtok(b, " \t\n\r");
    while (token != NULL && NMots < 99) {
        Mots[NMots++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    Mots[NMots] = NULL;
    return NMots;
}

int main()
{
    majComInt();
    char hostname[256];
    gethostname(hostname, 256);
    char prompt[512];
    sprintf(prompt, "%s@%s$ ", getenv("USER"), hostname);

    signal(SIGINT, SIG_IGN);
    read_history(HIST_FILE);

    char *ligne, *segment, *ptr;
    while ((ligne = readline(prompt)) != NULL)
    {
        if (*ligne != '\0')
        {
            add_history(ligne);
            append_history(1, HIST_FILE);
        }

        ptr = ligne;
        while ((segment = strsep(&ptr, ";")) != NULL)
        {
            int n = analyseCom(segment);
            if (n > 0)
            {
                if (!execComInt(n, Mots))
                {
                    execComExt(Mots);
                }
            }
        }
        free(ligne);
    }
    write_history(HIST_FILE);
    return 0;
}