# TP_PROG
# Réalise par Ben Youssef Nour / Bouzidi Amira

# Question 1 GETTFTP
Ce code implémente le protocole TFTP (Trivial File Transfer Protocol) pour envoyer des données d'un serveur à un client. Voici comment il fonctionne :

     1/Paramètres d'entrée :
       Le programme prend en paramètres d'entrée l'adresse IP du serveur et le nom du fichier à récupérer.

     2/Configuration du socket :
       ->La structure addrinfo est configurée pour obtenir les informations d'adresse du serveur TFTP.
       ->getaddrinfo est utilisée pour obtenir ces informations en fonction de l'adresse IP du serveur et du port 1069 (port standard du protocole TFTP).

     3/Création du socket :
       ->socket crée un socket UDP pour la communication avec le serveur TFTP.

    4/Construction et envoi de la demande de fichier :
       ->Le programme construit un paquet TFTP de type demande de fichier (opcode 1) avec le nom du fichier et le mode de transfert (ici, "octet").
       ->Ce paquet est envoyé au serveur avec sendto.

    5/Réception et écriture des données du serveur :
       ->Le programme utilise une boucle infinie pour recevoir les données du serveur via recvfrom.
       ->Après la réception des données, le programme envoie un accusé de réception (ACK) confirmant la réception du bloc de données au serveur.
       ->Les données reçues (hormis les 4 premiers octets de l'en-tête) sont écrites dans un fichier local.

    6/Gestion des blocs et fin du transfert :
       ->Le programme vérifie que le bloc de données reçu correspond à celui attendu avant de l'écrire dans le fichier local.
       ->Le transfert se poursuit jusqu'à ce qu'un paquet de données reçu soit inférieur à 512 octets (indiquant la fin du transfert).
       ->Une fois le transfert terminé, le fichier est fermé, les ressources nettoyées et le programme se termine.
# Question 2 PUTTFTP
Ce code implémente le protocole TFTP (Trivial File Transfer Protocol) pour envoyer des données d'un client à un serveur. Voici comment il fonctionne :

    1/ Initialisation du client : Le client crée un socket UDP pour établir une connexion avec le serveur en utilisant socket() et getaddrinfo() pour obtenir les informations d'adresse du serveur.

    2/ Envoi de la demande d'écriture (WRQ) : Le client envoie un paquet de demande d'écriture (WRQ) au serveur via sendto(). Ce paquet contient le nom du fichier à écrire et le mode de transfert.

    3/ Ouverture du fichier : Le client ouvre le fichier à envoyer en mode lecture binaire ("rb") via fopen().

    4/ Préparation et envoi des blocs de données :
      -> Le client lit des blocs de données du fichier et les envoie au serveur sous forme de paquets UDP. Chaque paquet est constitué de :
      -> Un en-tête de 4 octets : Opcode 3 (données) et le numéro de bloc.
      -> Les données lues du fichier (512 octets maximum par paquet).
      -> Chaque bloc de données est envoyé individuellement au serveur via sendto().

    5/ Réception des ACK (accusés de réception) : Après l'envoi de chaque bloc, le client attend un accusé de réception (ACK) du serveur indiquant la réception réussie du bloc de données. Le client utilise recvfrom() pour attendre les ACK du serveur.

    6/ Contrôle de la transmission : Si le client reçoit un ACK correspondant au numéro de bloc envoyé, il envoie le bloc de données suivant. En cas d'ACK incorrect ou manquant, le client réessaie l'envoi du même bloc.

    7/ Fin du transfert de fichier : Lorsque le fichier est entièrement envoyé et que le dernier bloc est transmis avec succès, le client termine le transfert et ferme le fichier.
