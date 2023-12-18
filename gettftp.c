#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 516

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_IP> <filename>\n", argv[0]);
        return 1;
    }

    // Configuration des paramètres pour l'obtention d'informations d'adresse
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_protocol = IPPROTO_UDP; 
    
    // Obtention des informations d'adresse pour le serveur
    int a = getaddrinfo(argv[1], "1069", &hints, &res);
    if (a != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(a));
        return 1;
    }

    // Création du socket
    int sck = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sck == -1) {
        perror("socket");
        return 1;
    }

    // Construction du paquet TFTP de demande de fichier
    char buffer[MAX_BUFFER_SIZE];
    buffer[0] = 0; 
    buffer[1] = 1;
    char *filename = argv[2];
    strcpy(&buffer[2], filename);
    strcpy(&buffer[2 + strlen(filename) + 1], "octet"); // Mode de transfert octet

    int size = 2 + strlen(filename) + 1 + strlen("octet") + 1;

    // Envoi de la demande de fichier au serveur
    int sendSize = sendto(sck, buffer, size, 0, res->ai_addr, res->ai_addrlen);
    if (sendSize == -1) {
        perror("sendto");
        return 1;
    }
    printf("Sent request: %d bytes\n", sendSize);

    // Ouverture du fichier pour écrire les données reçues
    FILE *fileOut = fopen(filename, "wb");
    if (fileOut == NULL) {
        perror("fopen");
        return 1;
    }

    int blockCounter = 1; 

    // Boucle pour recevoir et écrire les données du serveur dans le fichier
    while (1) {
        struct sockaddr_storage server_addr;
        socklen_t addrsize = sizeof(server_addr);
        int recvSize = recvfrom(sck, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addrsize);
        if (recvSize == -1) {
            perror("recvfrom");
            return 1;
        }

        unsigned short blockId = (buffer[2] << 8) | buffer[3];

        // Envoi de l'ACK au serveur pour confirmer la réception du bloc de données
        buffer[0] = 0;
        buffer[1] = 4;
        buffer[2] = (blockCounter >> 8) & 0xFF;
        buffer[3] = blockCounter & 0xFF;

        int ackSize = sendto(sck, buffer, 4, 0, (struct sockaddr *)&server_addr, addrsize);
        if (ackSize == -1) {
            perror("sendto ACK");
            return 1;
        }
        
        // Vérification si le paquet reçu est complet
        if (recvSize < 4) {
            printf("Received incomplete packet. Exiting...\n");
            break;
        }

        // Écriture des données reçues dans le fichier
        if (blockId == blockCounter) {
            fwrite(&buffer[4], 1, recvSize - 4, fileOut);
            blockCounter++;
        } else {
            printf("Received unexpected block: %hu (expected: %d). Ignoring...\n", blockId, blockCounter);
        }

        // Vérification de la fin du transfert de fichier
        if (recvSize - 4 < 512) {
            printf("File transfer completed.\n");
            break;
        }
    }

    // Fermeture du fichier et nettoyage des ressources
    fclose(fileOut);
    freeaddrinfo(res);
    close(sck);
    return 0;
}

