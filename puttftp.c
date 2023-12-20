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
#define DATA_BLOCK_SIZE 512

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_IP> <filename>\n", argv[0]);
        return 1;
    }

    // Configuration des paramètres pour obtenir les informations d'adresse
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

    // Construction du paquet TFTP de demande d'écriture (WRQ)
    char buffer[MAX_BUFFER_SIZE];
    buffer[0] = 0;
    buffer[1] = 2; // Opcode 2 pour la demande d'écriture
    char *filename = argv[2];
    strcpy(&buffer[2], filename);
    strcpy(&buffer[2 + strlen(filename) + 1], "octet"); // Mode de transfert octet

    int size = 2 + strlen(filename) + 1 + strlen("octet") + 1;

    // Envoi de la demande d'écriture au serveur
    int sendSize = sendto(sck, buffer, size, 0, res->ai_addr, res->ai_addrlen);
    if (sendSize == -1) {
        perror("sendto");
        return 1;
    }
    printf("Sent write request: %d bytes\n", sendSize);

    FILE *fileOut = fopen(filename, "rb");
    if (fileOut == NULL) {
        perror("fopen");
        return 1;
    }

    unsigned short blockCounter = 0;

    while (1) {
        // Construction du bloc de données à envoyer
        unsigned char buffer[MAX_BUFFER_SIZE];
        buffer[0] = 0;
        buffer[1] = 3; // Opcode 3 pour les données
        buffer[2] = (blockCounter >> 8) & 0xFF;
        buffer[3] = blockCounter & 0xFF;

        // Lecture du fichier
        fseek(fileOut, blockCounter * DATA_BLOCK_SIZE, SEEK_SET);
        size_t bytesRead = fread(&buffer[4], 1, DATA_BLOCK_SIZE, fileOut);

        int dataSize = bytesRead + 4;

        // Envoi du bloc de données au serveur
        int sendSize = sendto(sck, buffer, dataSize, 0, res->ai_addr, res->ai_addrlen);
        if (sendSize == -1) {
            perror("sendto data");
            return 1;
        }

        // Réception de l'ACK du serveur
        struct sockaddr_storage server_addr;
        socklen_t addrsize = sizeof(server_addr);
        int recvSize = recvfrom(sck, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addrsize);
        if (recvSize == -1) {
            perror("recvfrom");
            return 1;
        }

        // Vérification de l'ACK reçu
        unsigned short blockId = (buffer[2] << 8) | buffer[3];

        if (blockId == blockCounter) {
            printf("Received ACK for block: %hu\n", blockCounter);
        } else {
            printf("Received unexpected ACK: %hu (expected: %hu). Retrying...\n", blockId, blockCounter);
            continue;
        }

        // Vérification de la fin du transfert de fichier
        if (bytesRead < DATA_BLOCK_SIZE) {
            printf("File transfer completed.\n");
            break;
        }

        blockCounter++;
    }

    // Fermeture du fichier et libération des ressources
    fclose(fileOut);
    freeaddrinfo(res);
    close(sck);
    return 0;
}

