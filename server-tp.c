#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 5000
#define BACKLOG 10
#define BUFFER_SIZE 1024

void procesar_comando(int clientfd, char *buffer);

int main(void) {
    // Crear socket TCP
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("ERROR: creando socket.");
        exit(EXIT_FAILURE);
    }

    // Configurar IP y puerto
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr) <= 0) {
        perror("ERROR: IP inválida con inet_pton.");
        exit(EXIT_FAILURE);
    }

    // Asociar socket a IP:PUERTO
    if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("ERROR: bind() falló.");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(s, BACKLOG) == -1) {
        perror("ERROR: listen() falló.");
        exit(EXIT_FAILURE);
    }

    // Bucle principal
    while (1) {
        socklen_t addr_len = sizeof(struct sockaddr_in);
        struct sockaddr_in clientaddr;
        int clientfd;

        printf("\n[+] Esperando conexión en puerto %d...\n", PORT);

        clientfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len);
        if (clientfd == -1) {
            perror("ERROR: al aceptar conexión");
            continue;
        }

        char ipClient[32];
        inet_ntop(AF_INET, &clientaddr.sin_addr, ipClient, sizeof(ipClient));
        printf("[+] Conexión aceptada desde %s\n", ipClient);

        // Leer comando
        char buffer[BUFFER_SIZE];
        int n = read(clientfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) {
            perror("ERROR: leyendo del socket");
            close(clientfd);
            continue;
        }

        buffer[n] = '\0';
        printf("[>] Comando recibido: %s\n", buffer);

        procesar_comando(clientfd, buffer);

        close(clientfd);
        printf("[x] Conexión cerrada con %s\n", ipClient);
    }

    close(s);
    return EXIT_SUCCESS;
}

void procesar_comando(int clientfd, char *buffer) {
    char *cmd = strtok(buffer, " \n");
    char *key = strtok(NULL, " \n");
    char *value = strtok(NULL, "\n");

    if (!cmd || !key) {
        write(clientfd, "ERROR\n", 6);
        return;
    }

    if (strcmp(cmd, "SET") == 0) {
        int fd = open(key, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("ERROR: abriendo archivo para SET");
            write(clientfd, "ERROR\n", 6);
            return;
        }

        if (value && write(fd, value, strlen(value)) < 0) {
            perror("ERROR: escribiendo archivo en SET");
            write(clientfd, "ERROR\n", 6);
            close(fd);
            return;
        }

        close(fd);
        write(clientfd, "OK\n", 3);
    }

    else if (strcmp(cmd, "GET") == 0) {
        int fd = open(key, O_RDONLY);
        if (fd < 0) {
            perror("ERROR: abriendo archivo en GET");
            write(clientfd, "NOTFOUND\n", 9);
            return;
        }

        write(clientfd, "OK\n", 3);

        char filebuf[BUFFER_SIZE];
        int bytes;
        while ((bytes = read(fd, filebuf, BUFFER_SIZE)) > 0) {
            if (write(clientfd, filebuf, bytes) < 0) {
                perror("ERROR: enviando datos al cliente en GET");
                break;
            }
        }

        write(clientfd, "\n", 1);
        close(fd);
    }

    else if (strcmp(cmd, "DEL") == 0) {
        if (unlink(key) == -1 && errno != ENOENT) {
            perror("ERROR: borrando archivo en DEL");
            write(clientfd, "ERROR\n", 6);
            return;
        }

        write(clientfd, "OK\n", 3);
    }

    else {
        write(clientfd, "ERROR\n", 6);
    }
}
