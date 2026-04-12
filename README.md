NOM: LAVIEVILLE
PRENOM: Benoit

# Projet Biceps - TP3 OS User 

Ce TP implémente le protocole BEUIP en utilisant une architecture multi-threadée.

## Structure du code
- **biceps.c** : Contient l'interpréteur de commandes et la gestion des commandes internes via un dispatcher. Il gère l'interface utilisateur et la saisie via la bibliothèque readline.
- **server_thread.c** : Implémentation du serveur UDP tournant dans un thread séparé. Il gère la détection automatique des interfaces réseau (getifaddrs) et la réception des paquets en arrière-plan.
- **creme.c** : Librairie "Commandes Rapides pour l'Envoi de Messages Evolués". Elle gère la structure de données en liste chaînée (ordonnée alphabétiquement) ainsi que les primitives d'envoi et de réception réseau.
- **creme.h** : Définitions des structures, des constantes (PORT 9998, BROADCAST_ADDR) et prototypes des fonctions de la librairie.

## Détails et architecture

### 1-Gestion de la concurrence
Le programme utilise des Mutex POSIX (`mutex_table`) pour protéger la liste chaînée des contacts. Comme cette ressource est partagée entre le thread principal (affichage/envoi) et le thread serveur (mise à jour lors des réceptions), les locks sont placés directement dans les fonctions de `creme.c` pour éviter les conditions de concurrence.

### 2-Réseau et détection automatique
Le serveur utilise la fonction `getifaddrs()` pour identifier dynamiquement les adresses de broadcast des interfaces réseau actives. Cela permet au programme de fonctionner sur différents réseaux sans configuration manuelle. En cas d'échec de la détection, une adresse est définie via la constante `BROADCAST_ADDR`.

### 3-Arrêt propre et gestion de la mémoire
Afin de permettre un arrêt propre du thread serveur sans blocage, la socket UDP est configurée avec un timeout `SO_RCVTIMEO`. Cela permet au serveur de vérifier régulièrement l'état de la variable `serveur_en_cours` et de libérer ses ressources. Une fonction `viderListe()` est appelée à la fermeture pour libérer toute la mémoire allouée dynamiquement, garantissant aucune fuite mémoire.

## Instructions de compilation
- `make` : Compile le projet et génère l'exécutable `biceps`.
- `make memory-leak` : Compile une version spécifique pour Valgrind nommée `biceps-memory-leaks` avec les options `-g -O0`.
- `make clean` : Supprime tous les fichiers objets et binaires.

## Fonctionnalités implémentées
- Lancement/Arrêt propre du serveur : `beuip start <pseudo>` et `beuip stop`.
- Annuaire dynamique : `beuip list` affiche les utilisateurs présents au format `IP : pseudo`.
- Communication : `beuip message <pseudo> <msg>` pour un message privé et `beuip message all <msg>` pour une diffusion générale.
- Gestion des accès concurrents : Utilisation de Mutex POSIX pour protéger la liste chaînée partagée entre les threads.